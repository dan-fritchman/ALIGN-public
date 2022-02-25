import logging
import re
import pathlib
import json
import copy
import collections
from collections import defaultdict
from itertools import chain

#from .. import PnR

from .render_placement import gen_placement_verilog, scale_placement_verilog, gen_boxes_and_hovertext, standalone_overlap_checker, scalar_rational_scaling, round_to_angstroms
from .checker import check_placement, check_place_on_grid
from ..gui.mockup import run_gui
from ..schema.hacks import List, FormalActualMap, VerilogJsonTop, VerilogJsonModule
from .hpwl import calculate_HPWL_from_placement_verilog_d, gen_netlist

from .grid_constraints import gen_constraints

import math

logger = logging.getLogger(__name__)

def place( *, DB, opath, fpath, numLayout, effort, idx, lambda_coeff, select_in_ILP, seed, use_analytical_placer, modules_d=None, ilp_solver, place_on_grid_constraints_json, run_cap_placer):

    logger.info(f'Starting bottom-up placement on {DB.hierTree[idx].name} {idx}')

    current_node = DB.CheckoutHierNode(idx,-1)

    DB.AddingPowerPins(current_node)

    if run_cap_placer:
        PRC = PnR.Placer_Router_Cap_Ifc(opath,fpath,current_node,DB.getDrc_info(),DB.checkoutSingleLEF(),1,6)

    hyper = PnR.PlacerHyperparameters()
    # Defaults; change (and uncomment) as required
    hyper.T_INT = 0.5  # Increase for denormalized decision criteria
    hyper.T_MIN = 0.05
    hyper.ALPHA = math.exp(math.log(hyper.T_MIN/hyper.T_INT)/1e4)
    # hyper.T_MIN = hyper.T_INT*(hyper.ALPHA**1e4)    # 10k iterations
    # hyper.ALPHA = 0.99925
    # hyper.max_init_trial_count = 10000
    # hyper.max_cache_hit_count = 10
    hyper.SEED = seed  # if seed==0, C++ code will use its default value. Else, C++ code will use the provided value.
    # hyper.COUNT_LIMIT = 200
    # hyper.select_in_ILP = False
    hyper.ilp_solver = 0 if ilp_solver == 'symphony' else 1
    hyper.LAMBDA = lambda_coeff
    hyper.use_analytical_placer = use_analytical_placer

    hyper.place_on_grid_constraints_json = place_on_grid_constraints_json

    if modules_d is not None:
        hyper.use_external_placement_info = True
        hyper.placement_info_json = json.dumps(modules_d, indent=2)

    curr_plc = PnR.PlacerIfc( current_node, numLayout, opath, effort, DB.getDrc_info(), hyper)

    actualNumLayout = curr_plc.getNodeVecSize()

    if actualNumLayout != numLayout:
        logger.debug( f'Placer did not provide numLayout ({numLayout} > {actualNumLayout}) layouts for {DB.hierTree[idx].name}')

    for lidx in range(actualNumLayout):
        node = curr_plc.getNode(lidx)
        if node.Guardring_Consts:
            logger.info( f'Running guardring flow')
            PnR.GuardRingIfc( node, DB.checkoutSingleLEF(), DB.getDrc_info(), fpath)
        DB.Extract_RemovePowerPins(node)
        DB.CheckinHierNode(idx, node)

    DB.hierTree[idx].numPlacement = actualNumLayout

def subset_verilog_d( verilog_d, nm):
    # Should be an abstract verilog_d; no concrete_instance_names

    for module in verilog_d['modules']:
        for instance in module['instances']:
            assert 'concrete_template_name' not in instance, (instance, module)
            assert 'abstract_template_name' in instance, (instance, module)

    modules = { module['name'] : module for module in verilog_d['modules']}

    found_modules = set()
    def aux( module_name):
        found_modules.add( module_name)
        if module_name in modules:
            for instance in modules[module_name]['instances']:        
                atn = instance['abstract_template_name']
                aux( atn)

    aux(nm)

    new_verilog_d = copy.deepcopy(verilog_d)

    new_modules = []
    for module in new_verilog_d['modules']:
        if module['name'] in found_modules:
            new_modules.append( module)
    
    new_verilog_d['modules'] = new_modules

    return new_verilog_d

def gen_leaf_bbox_and_hovertext( ctn, p):
    #return (p, list(gen_boxes_and_hovertext( placement_verilog_d, ctn)))
    d = { 'width': p[0], 'height': p[1]}
    return d, [ ((0, 0)+p, f'{ctn}<br>{0} {0} {p[0]} {p[1]}', True, 0, False)], None

def scale_and_check_placement(*, placement_verilog_d, concrete_name, scale_factor, opath, placement_verilog_alternatives, is_toplevel):
    (pathlib.Path(opath) / f'{concrete_name}.placement_verilog.json').write_text(placement_verilog_d.json(indent=2,sort_keys=True))
    scaled_placement_verilog_d = scale_placement_verilog( placement_verilog_d, scale_factor)
    (pathlib.Path(opath) / f'{concrete_name}.scaled_placement_verilog.json').write_text(scaled_placement_verilog_d.json(indent=2,sort_keys=True))
    standalone_overlap_checker( scaled_placement_verilog_d, concrete_name)
    #Comment out the next two calls to disable checking (possibly to use the GUI to visualize the error.)
    check_placement( scaled_placement_verilog_d, scale_factor)
    if is_toplevel:
        check_place_on_grid(scaled_placement_verilog_d, concrete_name, opath)
    placement_verilog_alternatives[concrete_name] = scaled_placement_verilog_d

def per_placement( placement_verilog_d, *, hN, concrete_top_name, scale_factor, gui, opath, tagged_bboxes, leaf_map, placement_verilog_alternatives, is_toplevel):
    if hN is not None:
        abstract_name = hN.name
        concrete_names = { m['concrete_name'] for m in placement_verilog_d['modules'] if m['abstract_name'] == abstract_name}
        assert len(concrete_names) == 1
        concrete_name = next(iter(concrete_names))
    else:
        concrete_name = concrete_top_name

    if not gui:
        logger.info( f'Working on {concrete_name}')

    scale_and_check_placement( placement_verilog_d=placement_verilog_d, concrete_name=concrete_name, scale_factor=scale_factor, opath=opath, placement_verilog_alternatives=placement_verilog_alternatives, is_toplevel=is_toplevel)
    

    nets_d = gen_netlist( placement_verilog_d, concrete_name)
    hpwl_alt = calculate_HPWL_from_placement_verilog_d( placement_verilog_d, concrete_name, nets_d, skip_globals=True)

    if hN is not None:
        if hpwl_alt != hN.HPWL_extend:
            logger.warning( f'hpwl: locally computed from netlist {hpwl_alt}, placer computed {hN.HPWL_extend} differ!')
        else:
            logger.debug( f'hpwl: locally computed from netlist {hpwl_alt}, placer computed {hN.HPWL_extend} are equal!')

    if gui:

        def r2wh( r):
            return (round_to_angstroms(r[2]-r[0]), round_to_angstroms(r[3]-r[1]))

        # placement_verilog_d is in hN units
        gui_scaled_placement_verilog_d = scale_placement_verilog( placement_verilog_d, 0.001)

        modules = { x['concrete_name']: x for x in gui_scaled_placement_verilog_d['modules']}

        p = r2wh(modules[concrete_name]['bbox'])

        if hN is not None:
            if hpwl_alt != hN.HPWL_extend:
                logger.warning( f'hpwl: locally computed from netlist {hpwl_alt}, placer computed {hN.HPWL_extend} differ!')

        reported_hpwl = hpwl_alt / 2000

        cost, constraint_penalty, area_norm, hpwl_norm = 0, 0, 0, 0
        if hN is not None:
            cost, constraint_penalty = hN.cost, hN.constraint_penalty
            area_norm, hpwl_norm = hN.area_norm, hN.HPWL_norm

        d = { 'width': p[0], 'height': p[1],
              'hpwl': reported_hpwl, 'cost': cost,
              'constraint_penalty': constraint_penalty,
              'area_norm': area_norm, 'hpwl_norm': hpwl_norm
        }

        logger.debug( f"Working on {concrete_name}: {d}")

        tagged_bboxes[abstract_name][concrete_name] = d, list(gen_boxes_and_hovertext( gui_scaled_placement_verilog_d, concrete_name, nets_d)), nets_d

        leaves  = { x['concrete_name']: x for x in gui_scaled_placement_verilog_d['leaves']}

        # construct set of abstract_template_names
        atns = defaultdict(set)

        for module in gui_scaled_placement_verilog_d['modules']:
            for instance in module['instances']:
                if 'abstract_template_name' in instance:
                    atn = instance['abstract_template_name'] 
                    if 'concrete_template_name' in instance:
                        ctn = instance['concrete_template_name']
                        if ctn in leaves:
                            atns[atn].add((ctn, r2wh(leaves[ctn]['bbox'])))

        # Hack to get CC capacitors because they are missing from gdsData2 above
        # Can be removed when CC capacitor generation is moved to correct spot in flow
        for atn, v in atns.items():
            for (ctn, p) in v:
                if ctn in leaf_map[atn]:
                    assert leaf_map[atn][ctn][0] == { 'width': p[0], 'height': p[1]}, (atn,ctn,leaf_map[atn][ctn][0], p)
                else:
                    leaf_map[atn][ctn] = gen_leaf_bbox_and_hovertext( ctn, p)




def gen_leaf_map(*, DB, gui):
    leaf_map = defaultdict(dict)
    if gui:
        # Get all the leaf cells sizes; still doesn't get the CC capacitors
        for atn, gds_lst in DB.gdsData2.items():
            ctns = [str(pathlib.Path(fn).stem) for fn in gds_lst]
            for ctn in ctns:
                if ctn in DB.lefData:
                    lef = DB.lefData[ctn][0]
                    p = scalar_rational_scaling(lef.width,mul=0.001,div=2), scalar_rational_scaling(lef.height,mul=0.001,div=2)
                    if ctn in leaf_map[atn]:
                        assert leaf_map[atn][ctn][0] == p, (leaf_map[atn][ctn][0], p)
                    else:
                        leaf_map[atn][ctn] = gen_leaf_bbox_and_hovertext( ctn, p)

                else:
                    logger.error( f'LEF for concrete name {ctn} (of {atn}) missing.')
    
    return leaf_map



def process_placements(*, DB, verilog_d, gui, lambda_coeff, scale_factor, reference_placement_verilog_d, concrete_top_name, opath):
    leaf_map = gen_leaf_map(DB=DB, gui=gui)
    tagged_bboxes = defaultdict(dict)

    placement_verilog_alternatives = {}

    TraverseOrder = DB.TraverseHierTree()

    for idx in TraverseOrder:
        is_toplevel = idx == TraverseOrder[-1]

        # Restrict verilog_d to include only sub-hierachies of the current name
        s_verilog_d = subset_verilog_d( verilog_d, DB.hierTree[idx].name)

        for sel in range(DB.hierTree[idx].numPlacement):
            # create new verilog for each placement
            hN = DB.CheckoutHierNode( idx, sel)
            placement_verilog_d = gen_placement_verilog( hN, idx, sel, DB, s_verilog_d)
            per_placement( placement_verilog_d, hN=hN, concrete_top_name=concrete_top_name, scale_factor=scale_factor, gui=gui, opath=opath, tagged_bboxes=tagged_bboxes, leaf_map=leaf_map, placement_verilog_alternatives=placement_verilog_alternatives, is_toplevel=is_toplevel)

    # hack for a reference placement_verilog_d

    if reference_placement_verilog_d is not None:
        scaled_placement_verilog_d = VerilogJsonTop.parse_obj(reference_placement_verilog_d)
        # from layers.json units to hN units (loss of precision can happen here)
        placement_verilog_d = scale_placement_verilog( scaled_placement_verilog_d, scale_factor, invert=True)

        per_placement( placement_verilog_d, hN=None, concrete_top_name=concrete_top_name, scale_factor=scale_factor, gui=gui, opath=opath, tagged_bboxes=tagged_bboxes, leaf_map=leaf_map, placement_verilog_alternatives=placement_verilog_alternatives, is_toplevel=True)

        # placement_verilog_alternatives is in layers.json units

    placements_to_run = None
    if gui:
        tagged_bboxes.update( leaf_map)
        top_level = DB.hierTree[TraverseOrder[-1]].name

        print( f"Press Ctrl-C to end the GUI interaction. If current selection is a toplevel placement, the routing engine will be called on that placement. If the current selection is not toplevel (an intermediate hierarchy or a leaf), the router call will be skipped.")

        selected_concrete_name = run_gui( tagged_bboxes=tagged_bboxes, module_name=top_level, lambda_coeff=lambda_coeff)

        # Don't like name hacking; make we can do this another way
        p = re.compile( r'^(\S+)_(\d+)$')

        placements_to_run = []
        m = p.match(selected_concrete_name)
        if m:
            if m.groups()[0] == top_level:
                sel = int(m.groups()[1])
                placements_to_run = [(sel,placement_verilog_alternatives[selected_concrete_name])]
        else:
            if selected_concrete_name in placement_verilog_alternatives:
                placements_to_run = [(None,placement_verilog_alternatives[selected_concrete_name])]                

    return placements_to_run, placement_verilog_alternatives


def hierarchical_place(*, DB, opath, fpath, numLayout, effort, verilog_d,
                       gui, lambda_coeff, scale_factor,
                       reference_placement_verilog_d, concrete_top_name, select_in_ILP, seed, use_analytical_placer, ilp_solver, primitives,
                       run_cap_placer):

    logger.info(f'Calling hierarchical_place with {"existing placement" if reference_placement_verilog_d is not None else "no placement"}')

    if reference_placement_verilog_d:
        #
        # Need to do this until we fix the PnR set placement code
        #    scales by 1 if scale_factor is 1, by 10 if scale_factor is 10 (2* compenstates for the automatic divide by 2 in scale_placement_verilog)
        #
        hack_placement_verilog_d = scale_placement_verilog( reference_placement_verilog_d, 2*scale_factor, invert=True)        

        modules = collections.defaultdict(list)
        for m in hack_placement_verilog_d['modules']:
            modules[m['abstract_name']].append(m)

    grid_constraints = {}

    frontier = {}

    assert verilog_d is not None

    for idx in DB.TraverseHierTree():

        json_str = json.dumps([{'concrete_name': k, 'constraints': v} for k, v in grid_constraints.items()], indent=2)

        modules_d = None
        if reference_placement_verilog_d is not None:
            modules_d = modules[DB.hierTree[idx].name]

        place(DB=DB, opath=opath, fpath=fpath, numLayout=numLayout, effort=effort, idx=idx,
              lambda_coeff=lambda_coeff, select_in_ILP=select_in_ILP,
              seed=seed, use_analytical_placer=use_analytical_placer,
              modules_d=modules_d, ilp_solver=ilp_solver, place_on_grid_constraints_json=json_str,
              run_cap_placer=run_cap_placer)

        # for each layout, generate a placement_verilog_d, make sure the constraints are attached to the leaves, then generate the restrictions
        # convert the restrictions into the form needed for the subsequent placements

        if primitives is not None:
            # Restrict verilog_d to include only sub-hierachies of the current name
            s_verilog_d = subset_verilog_d( verilog_d, DB.hierTree[idx].name)

            frontier = {}

            for sel in range(DB.hierTree[idx].numPlacement):
                # create new verilog for each placement
                hN = DB.CheckoutHierNode( idx, sel)
                placement_verilog_d = gen_placement_verilog( hN, idx, sel, DB, s_verilog_d)
                # hN units
                scaled_placement_verilog_d = scale_placement_verilog( placement_verilog_d, scale_factor)
                # layers.json units (*5 if in anstroms)

                for leaf in scaled_placement_verilog_d['leaves']:
                    ctn = leaf['concrete_name']
                    if ctn not in primitives:
                        continue # special case capacitors

                    primitive = primitives[ctn]
                    if 'metadata' in primitive and 'constraints' in primitive['metadata']:
                        if 'constraints' not in leaf:
                            leaf['constraints'] = []

                        leaf['constraints'].extend(constraint for constraint in primitive['metadata']['constraints'])

                top_name = f'{hN.name}_{sel}'
                gen_constraints(scaled_placement_verilog_d, top_name)
                top_module = next(iter([module for module in scaled_placement_verilog_d['modules'] if module['concrete_name'] == top_name]))

                frontier[top_name] = [constraint.dict() for constraint in top_module['constraints'] if constraint.constraint == 'place_on_grid']

                for constraint in frontier[top_name]:
                    assert constraint['constraint'] == 'place_on_grid'
                    # assert constraint['ored_terms'], f'No legal grid locations for {top_name} {constraint}'
                    # Warn now and fail at the end for human-readable error message
                    if not constraint['ored_terms']:
                        logger.warning(f'No legal grid locations for {top_name} {constraint}')

            grid_constraints.update(frontier)

    placements_to_run, placement_verilog_alternatives = process_placements(DB=DB, verilog_d=verilog_d, gui=gui,
                                                                           lambda_coeff=lambda_coeff, scale_factor=scale_factor,
                                                                           reference_placement_verilog_d=reference_placement_verilog_d,
                                                                           concrete_top_name=concrete_top_name,
                                                                           opath=opath)

    if placements_to_run is not None:
        if placements_to_run:
            assert len(placements_to_run) == 1
            _, placement_verilog_d = placements_to_run[0]

            with open("__placement_verilog_d", "wt") as fp:
                fp.write(placement_verilog_d.json(indent=2))

            # Observation from looking at this file
            # In the set_bounding_box constraint, top-level cells are named instances (should be concrete_template)
            # There might be an issue with name space collisions if an instance and template are named the same
            # there is a flag to distinguish between instances and template; we should probably just rename instance
            # to something more generic like 'nm'.

            #
            # Build DB objects from placement_verilog_d
            #
            # create new blocks that are clones of existing blocks
            # 
            # Add in placement information

    if placements_to_run is not None:
        placements_to_run = [p[0] for p in placements_to_run]
        if placements_to_run == [None]: # Fix corner case until the new scheme works
            placements_to_run = None

    return placements_to_run, placement_verilog_alternatives


