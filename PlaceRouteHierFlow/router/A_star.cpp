#include "A_star.h"

A_star::A_star(Grid& grid, bool shielding):drc_info(grid.drc_info){

  this->path_number=1;
  source = grid.Source;
  dest = grid.Dest;
  this->shielding = shielding;

};


bool A_star::FindFeasiblePath(Grid& grid, int pathNo, int left_up, int right_down) {

  
  bool mark=false;
  for(int i =0;i<pathNo;++i){
    
     std::cout<<"Path No "<<pathNo<<" current path index "<<i<<std::endl;

     std::vector<std::vector<int> > temp_path;
     
     std::cout<<"start A_star "<<std::endl;

     temp_path = A_star_algorithm(grid, left_up, right_down);// grid.Source grid.dest
     
     std::cout<<"end A_star"<<std::endl; 

     if((int)temp_path.size()>0) {
       Path = temp_path;
     mark=true;
     } else {
       mark=(mark or false);
       std::cout<<"Router-Warning: feasible path might not be found\n";
     }
  }
  return mark;

}

std::vector<std::vector<RouterDB::Metal> > A_star::ConvertPathintoPhysical(Grid& grid){

  std::vector<std::vector<RouterDB::Metal> > Phsical_Path;
  for(int i= 0; i<(int) Path.size();++i){
      std::vector<RouterDB::Metal> temp_physical_path;
      //int start_index = 0;
      //int end_index = 0;
      int flag_start_write = 1;
      //int flag_end_write = 0;
      RouterDB::point temp_point;
      RouterDB::Metal temp_metal;
      for(int j=0;j<(int) Path[i].size();++j){
          if(flag_start_write == 1){
              temp_metal.LinePoint.clear();
              temp_metal.MetalIdx = grid.vertices_total[Path[i][j]].metal;
              temp_point.x = grid.vertices_total[Path[i][j]].x;
              temp_point.y = grid.vertices_total[Path[i][j]].y;
              temp_metal.LinePoint.push_back(temp_point);
              flag_start_write = 0;
            }
           if(j<(int) Path[i].size()-1 and grid.vertices_total[Path[i][j]].metal!=grid.vertices_total[Path[i][j+1]].metal){
              flag_start_write = 1;
              temp_point.x = grid.vertices_total[Path[i][j]].x;
              temp_point.y = grid.vertices_total[Path[i][j]].y;
              temp_metal.LinePoint.push_back(temp_point);
              temp_metal.width = grid.drc_info.Metal_info[grid.vertices_total[Path[i][j]].metal].width;
              temp_physical_path.push_back(temp_metal);
            }
            if(j==(int) Path[i].size()-1 and flag_start_write == 0){

              flag_start_write = 1;
              temp_point.x = grid.vertices_total[Path[i][j]].x;
              temp_point.y = grid.vertices_total[Path[i][j]].y;
              temp_metal.LinePoint.push_back(temp_point);
              temp_metal.width = grid.drc_info.Metal_info[grid.vertices_total[Path[i][j]].metal].width;
              temp_physical_path.push_back(temp_metal);
            }
            
         }
      Phsical_Path.push_back(temp_physical_path);
     }

  return Phsical_Path;
};

int A_star::Manhattan_distan(int sindex, Grid& grid){

  std::set<int> Mdis;

  for(int i=0;i<(int) dest.size();i++){

      int temp_dis = abs(grid.vertices_total[sindex].x - grid.vertices_total[dest[i]].x)+abs(grid.vertices_total[sindex].y - grid.vertices_total[dest[i]].y);
      Mdis.insert(temp_dis);

     }

  std::set<int>::iterator it;

  it=Mdis.begin();

  int dis = *it;

  return dis;
  
};

void A_star::initial_source(Grid& grid, std::set<std::pair<int,int>, RouterDB::pairComp>& L_list){
  
  for(int i=0;i<(int)source.size();i++){

         int Mdis = Manhattan_distan(source[i], grid);
         grid.vertices_total[source[i]].Cost = 0;
         int dis = grid.vertices_total[source[i]].Cost + Mdis;
         std::pair<int,int> temp_pair;
         temp_pair.first = dis;
         temp_pair.second = source[i];
         L_list.insert(temp_pair);

     }

};

bool A_star::found_near_node(int current_node, Grid &grid, std::vector<int> &candidate_node){

    
    std::vector<int> north_node, south_node, east_node, west_node, up_node, down_node;
    bool north_found, south_found, east_found, west_found, up_found, down_found;

    //std::cout<<"expand node checkout point1"<<std::endl;
    north_found = expand_node(grid.vertices_total[current_node].north, north_node, grid);
    //std::cout<<"expand node checkout point2"<<std::endl;
    south_found = expand_node(grid.vertices_total[current_node].south, south_node, grid);
    //std::cout<<"expand node checkout point3"<<std::endl;
    east_found = expand_node(grid.vertices_total[current_node].east, east_node, grid);
    //std::cout<<"expand node checkout point4"<<std::endl;
    west_found = expand_node(grid.vertices_total[current_node].west, west_node, grid);
    //std::cout<<"expand node checkout point5"<<std::endl;

    if(north_found){
       for(int i=0;i<(int)north_node.size();i++){
         candidate_node.push_back(north_node[i]);
       }
      }
    //std::cout<<"expand node checkout point8"<<std::endl;
    if(south_found){
       for(int i=0;i<(int)south_node.size();i++){
         candidate_node.push_back(south_node[i]);
       }
      }
    //std::cout<<"expand node checkout point9"<<std::endl;
    if(west_found){
       for(int i=0;i<(int)west_node.size();i++){
         candidate_node.push_back(west_node[i]);
       }
      }
    //std::cout<<"expand node checkout point10"<<std::endl;
    if(east_found){
       for(int i=0;i<(int)east_node.size();i++){
         candidate_node.push_back(east_node[i]);
       }
      }
    //std::cout<<"expand node checkout point11"<<std::endl;
    if(up_found){
       for(int i=0;i<(int)up_node.size();i++){
         candidate_node.push_back(up_node[i]);
       }
      }
    //std::cout<<"expand node checkout point12"<<std::endl;
    if(down_found){
       for(int i=0;i<(int)down_node.size();i++){
         candidate_node.push_back(down_node[i]);
       }
      }
    //std::cout<<"expand node checkout point13"<<std::endl;
    if((int)candidate_node.size()>0){
       //std::cout<<"candidate node Found"<<std::endl;
       return true;
      }else{
       //std::cout<<"candidate node not Found"<<std::endl;
       return false;
      }
};

bool A_star::expand_node(std::vector<int> &direction, std::vector<int> &temp_node, Grid &grid){

  for(int i=0;i<(int)direction.size();i++){
 
       if(grid.vertices_total[direction[i]].active and grid.vertices_total[direction[i]].Cost==-1){
       temp_node.push_back(direction[i]);
       }
     }

  if((int)temp_node.size()>0){
    return true;
    }else{
    return false;
    }

};



int A_star::trace_back_node(int current_node, Grid& grid){

  int first_node_same_layer = current_node;

  bool trace_back_flag = true;

  int dummy_node = current_node;

  while(trace_back_flag){

    int last_node = grid.vertices_total[dummy_node].parent;

    if(last_node<0 or last_node>=grid.vertices_total.size()){
      trace_back_flag = false;
    }else if(grid.vertices_total[last_node].metal == grid.vertices_total[dummy_node].metal){
      first_node_same_layer = last_node;
      dummy_node = last_node;
    }else{
      trace_back_flag = false;
    }

  }

  return first_node_same_layer;


};

bool A_star::CheckExendable_With_Certain_Length(int first_node_same_layer,int current_node,int length,int minL,Grid &grid){

  int half_minL = ceil( ( (double) minL -  (double) length) / 2 );

  bool feasible = true;
  
  int first_direction = 0;

  int current_direction = 0;
 
  if(first_node_same_layer<=current_node){

     first_direction = -1;
     current_direction = 1;

  }else{

     first_direction = 1;
     current_direction = -1;

  }

  bool search_flag = true;
  int culmulated_length = 0;
  int dummy_node = first_node_same_layer;
  while(search_flag){
     if(culmulated_length>=half_minL){
        search_flag = false;
     }else{
       int next_node = dummy_node + first_direction;
       if(next_node<0 or next_node>=grid.vertices_total.size() ) {
          search_flag = false;
          feasible = false;
       }else if(grid.vertices_total[next_node].active==0) {
          search_flag = false;
          feasible = false;
       }else if( (grid.vertices_total[next_node].x != grid.vertices_total[first_node_same_layer].x and grid.vertices_total[next_node].y != grid.vertices_total[first_node_same_layer].y) or grid.vertices_total[next_node].metal != grid.vertices_total[first_node_same_layer].metal ){
          search_flag = false;
          feasible = false;
       }else {
          culmulated_length = abs(grid.vertices_total[next_node].x-grid.vertices_total[first_node_same_layer].x) + abs( grid.vertices_total[next_node].y-grid.vertices_total[first_node_same_layer].y);
          dummy_node = next_node;
       }
     }
  }

  culmulated_length = 0;
  search_flag = true;
  dummy_node = current_node;
  while(search_flag){
     if(culmulated_length>=half_minL){
        search_flag = false;
     }else{
       int next_node = dummy_node + current_direction;
       if(next_node<0 or next_node>=grid.vertices_total.size() ) {
          search_flag = false;
          feasible = false;
       }else if(grid.vertices_total[next_node].active==0) {
          search_flag = false;
          feasible = false;
       }else if( (grid.vertices_total[next_node].x != grid.vertices_total[current_node].x and grid.vertices_total[next_node].y != grid.vertices_total[current_node].y) or grid.vertices_total[next_node].metal != grid.vertices_total[current_node].metal){
          search_flag = false;
          feasible = false;
       }else {
          culmulated_length = abs(grid.vertices_total[next_node].x-grid.vertices_total[current_node].x) + abs( grid.vertices_total[next_node].y-grid.vertices_total[current_node].y);
          dummy_node = next_node;
       }
     }
  }

  return feasible;

};

bool A_star::find_nodes_north(Grid& grid, int node, int number, std::vector<int>& temp_nodes){

  int interval_number = 1;
  temp_nodes.push_back(node);
  int current_node = -1;
  while(number!=0){

     int current_node = temp_nodes.back();
     int temp_number = interval_number;
     
     while(temp_number!=0){

        if(grid.vertices_total_full_connected[current_node].north.size()>0){ // vertices_total_full_connected // vertices_total
           n = grid.vertices_total_full_connected[current_node].north[0];
        }else{
           n = -1;
        }
        if(n==-1){
          return false;
        }else{
          current_node = n;
        }
        
        temp_number = temp_number - 1;
     }
     
     temp_nodes.push_back(n);

     number = number - 1;

  }

  reverse(temp_nodes.begin(),temp_nodes.end());
  temp_node.pop_back();
  return true;

};

bool A_star::find_nodes_east(Grid& grid, int node, int number, std::vector<int>& temp_nodes){

  int interval_number = 1;
  temp_nodes.push_back(node);
  int current_node = -1;
  while(number!=0){

     int current_node = temp_nodes.back();
     int temp_number = interval_number;
     
     while(temp_number!=0){

        if(grid.vertices_total_full_connected[current_node].east.size()>0){
           n = grid.vertices_total_full_connected[current_node].east[0];
        }else{
           n = -1;
        }
        if(n==-1){
          return false;
        }else{
          current_node = n;
        }
        
        temp_number = temp_number - 1;
     }
     
     temp_nodes.push_back(n);

     number = number - 1;

  }

  reverse(temp_nodes.begin(),temp_nodes.end());
  temp_node.pop_back();
  return true;

};

bool A_star::find_nodes_west(Grid& grid, int node, int number, std::vector<int>& temp_nodes){

  int interval_number = 1;
  temp_nodes.push_back(node);
  int current_node = -1;
  while(number!=0){

     int current_node = temp_nodes.back();
     int temp_number = interval_number;
     
     while(temp_number!=0){

        if(grid.vertices_total_full_connected[current_node].west.size()>0){
           n = grid.vertices_total_full_connected[current_node].west[0];
        }else{
           n = -1;
        }
        if(n==-1){
          return false;
        }else{
          current_node = n;
        }
        
        temp_number = temp_number - 1;
     }
     
     temp_nodes.push_back(n);

     number = number - 1;

  }

  reverse(temp_nodes.begin(),temp_nodes.end());
  temp_node.pop_back();
  return true;

};

bool A_star::find_nodes_south(Grid& grid, int node, int number, std::vector<int>& temp_nodes){

  int interval_number = 1;
  temp_nodes.push_back(node);
  int current_node = -1;
  while(number!=0){

     int current_node = temp_nodes.back();
     int temp_number = interval_number;
     
     while(temp_number!=0){

        if(grid.vertices_total_full_connected[current_node].south.size()>0){
           n = grid.vertices_total_full_connected[current_node].south[0];
        }else{
           n = -1;
        }
        if(n==-1){
          return false;
        }else{
          current_node = n;
        }
        
        temp_number = temp_number - 1;
     }
     
     temp_nodes.push_back(n);

     number = number - 1;

  }

  reverse(temp_nodes.begin(),temp_nodes.end());
  temp_node.pop_back();
  return true;

};

bool A_star::Check_Src_Dest(std::vector<int> &nodes, std::set<int> src_dest){


  for(int i=0;i<nodes.size();i++){
     if(src_dest.find(nodes[i])==src_dest.end()){
       return false;
     }
  }
  return true;

};

bool A_star::find_succsive_parallel_node(Grid& grid, int current_node, int left, int right, int mode, std::vector<int> &nodes, std::set<int> &src_index, std::set<int> &dest_index){

  bool exist = 0;
  if(drc_info.Metal_info[grid.vertices_total[current_node].metal].direct==1){//h

    vector<int> temp_nodes;
    int exist;
    if(mode==0){
      exist = find_nodes_south(grid, current_node, left, temp_nodes);
      exist = Check_Src_Dest(temp_nodes, src_index);
    }else{
      exist = find_nodes_west(grid, current_node, left, temp_nodes);
    }

    if(exist){
      nodes.insert(nodes.end(),temp_nodes.begin(),temp_nodes.end());
    }else{
     return false;
    }
    
  }else{

    vector<int> temp_nodes;
    int exist;
    if(mode==0){
      exist = find_nodes_west(grid, current_node, left, temp_nodes);
      exist = Check_Src_Dest(temp_nodes, src_index);
    }else{
      exist = find_nodes_south(grid, current_node, left, temp_nodes);
    }

    if(exist){
      nodes.insert(nodes.end(),temp_nodes.begin(),temp_nodes.end());
    }else{
     return false;
    }

  }

  nodes.push_back(current_node);


  if(drc_info.Metal_info[grid.vertices_total[current_node].metal].direct==1){//h

    vector<int> temp_nodes;
    int exist;
    if(mode==0){
      exist = find_nodes_north(grid, current_node, right, temp_nodes);
      exist = Check_Src_Dest(temp_nodes, dest_index);
    }else{
      exist = find_nodes_east(grid, current_node, right, temp_nodes);
    }

    if(exist){
      nodes.insert(nodes.end(),temp_nodes.begin(),temp_nodes.end());
    }else{
     return false;
    }
    
  }else{

    vector<int> temp_nodes;
    int exist;
    if(mode==0){
      exist = find_nodes_east(grid, current_node, right, temp_nodes);
      exist = Check_Src_Dest(temp_nodes, dest_index);
    }else{
      exist = find_nodes_north(grid, current_node, right, temp_nodes);
    }

    if(exist){
      nodes.insert(nodes.end(),temp_nodes.begin(),temp_nodes.end());
    }else{
     return false;
    }

  }

  return true;

};

bool A_star::parallel_routing(Grid& grid, int current_node, int next_node, int left, int right, std::set<int> &source_index, std::set<int> &dest_index, std::set<int> &src_index, std::set<int> &dest_index){

  std::vector<int> start_points;
  std::vector<int> end_points;
  bool found_s;
  bool found_e;  

  if(source_index.find(current_node)!=source_index.end()){
    int mode = 0; //succsive
    found_s = find_succsive_parallel_node(current_node, left, right, mode, start_points, src_index, dest_index);
  }else{
    int mode = 1; //parallel
    found_s = find_succsive_parallel_node(current_node, left, right, mode, start_points, src_index, dest_index);
  }

  if(dest_index.find(next_node)!=dest_index.end()){
    int mode = 0; //succsive
    found_e = find_succsive_arallel_node(next_node, left, right, mode, end_points, src_index, dest_index);
  }else{
    int mode = 1; //parallel
    found_e = find_succsive_parallel_node(next_node, left, right, mode, end_points, src_index, dest_index);
  }

  if(found_s and found_e){
     return L_shape_Connection_Check(start_points,end_points);
  }else{
    return false;
  }

};


bool A_star::L_shape_Connection(Grid& grid, std::vector<int> &start_points, std::vector<int> &end_points){

  for(int i=0;i<start_points;i++){

      int s_node = start_points[i];
      int e_node = end_points[i];
      bool connection = L_shape_Connection_Check(grid,s_node,e_node);
      if(!connection){return false;}

  }

  return true;

};


bool A_star::L_shape_Connection_Check(Grid& grid, int start_points, int end_points){

  std::vector<int> node_set_up;
  node_set_up.push_back(start_points);

  while(node_set_up.back()!=end_points){ // QQQ: might be stacked here

    int current_node = node_set_up.back();
    int x = grid.vertices_total[current_node].x - grid.vertices_total[end_points].x;
    if(x>0){x=1;}else{x=-1};
    int y = grid.vertices_total[current_node].y - grid.vertices_total[end_points].y;
    if(y>0){y=1;}else{y=-1};
    int metal = grid.vertices_total[current_node].metal - grid.vertices_total[end_points].metal;
    if(metal>0){metal=1;}else{metal=-1};
    int dummy_layer = 1; // go up
    int next = find_next_node(gird, current_node, x, y, layer, dummy_layer);
    if(next==-1){
      return false;
    }else if(next>0 and next< grid.vertices_total.size() ){
      grid.vertices_total[next].parent = current_node;
      node_set_up.push_back(next); 
    }else{
      std::cout<<"L shape connection check bug, next node is out of grid"<<std::endl;
      assert(0);
    }
    
  }


  std::vector<int> node_set_down;
  node_set_down.push_back(start_points);

  while(node_set_down.back()!=end_points){ // QQQ: might be stacked here

    int current_node = node_set_down.back();
    int x = grid.vertices_total[current_node].x - grid.vertices_total[end_points].x;
    if(x>0){x=1;}else{x=-1};
    int y = grid.vertices_total[current_node].y - grid.vertices_total[end_points].y;
    if(y>0){y=1;}else{y=-1};
    int metal = grid.vertices_total[current_node].metal - grid.vertices_total[end_points].metal;
    if(metal>0){metal=1;}else{metal=-1};
    int dummy_layer = -1; // go down
    int next = find_next_node(grid, current_node, x, y, layer, dummy_layer);
    if(next==-1){
      return false;
    }else if(next>0 and next< grid.vertices_total.size() ){
      grid.vertices_total[next].parent = current_node;
      node_set_down.push_back(next); 
    }else{
      std::cout<<"L shape connection check bug, next node is out of grid"<<std::endl;
      assert(0);
    }
    
  }

  
  bool extend_up = Extention_checks(grid, node_set_up);
  bool extend_down = Extention_checks(grid, node_set_down);

  bool activa_up = Check_activa_via_activa(grid, node_set_up);
  bool activa_down = Check_activa_via_activa(grid, node_set_down);

  if( (extend_up and activa_up) or (extend_down and activa_down)){
    return true;
  }else{
    return false;
  }

};

int A_star::find_next_node( Grid& grid, int current_node, int x, int y, int layer, int dummy_layer){

  int next_node = -1;

  if(drc_info.Metal_info[grid.vertices_total[current_node].metal].direct==1 and x!=0){//h
    next_node = current_node + x;
  }else if(drc_info.Metal_info[grid.vertices_total[current_node].metal].direct==1 and x==0 and layer!=0){
    if(layer>0){
      next_node = grid.vertices_total[current_node].up;
    }else{
      next_node = grid.vertices_total[current_node].down;
    }
  }else if(drc_info.Metal_info[grid.vertices_total[current_node].metal].direct==1 and x==0 and layer==0){
    if(dummy_layer>0){
      next_node = grid.vertices_total[current_node].up;
    }else{
      next_node = grid.vertices_total[current_node].down;
    }
  }else if(drc_info.Metal_info[grid.vertices_total[current_node].metal].direct==0 and y!=0){//h
    next_node = current_node + y;
  }else if(drc_info.Metal_info[grid.vertices_total[current_node].metal].direct==0 and y==0 and layer!=0){
    if(layer>0){
      next_node = grid.vertices_total[current_node].up;
    }else{
      next_node = grid.vertices_total[current_node].down;
    }
  }else if(drc_info.Metal_info[grid.vertices_total[current_node].metal].direct==0 and y==0 and layer==0){
    if(dummy_layer>0){
      next_node = grid.vertices_total[current_node].up;
    }else{
      next_node = grid.vertices_total[current_node].down;
    }
  }

  return next_node;


};

bool A_star::Check_activa_via_active(Grid& grid, std::vector<int> &nodes){

  for(int i=0;i<nodes.size();i++){

     int parent = grid.vertices_total[nodes[i]].parent;
     if(parent==-1){
        continue;
     }else if(parent <0 or parent> grid.vertices_total.size() -1){
        std::cout<<"Check active via active bug, parent out of grid"<<st::endl;
     }
     int parent_metal = grid.vertices_total[parent].metal;
     int current_metal = grid.vertices_total[nodes[i]].metal;
     if(parent_metal == current_metal and !grid.vertices_total[nodes[i]].active){
       return false;
     }else if(parent_metal > current_metal and (!grid.vertices_total[nodes[i]].active or !grid.vertices_total[nodes[i]].via_active_up)){
       return false;
     }else if(parent_metal < current_metal and (!grid.vertices_total[nodes[i]].active or !grid.vertices_total[nodes[i]].via_active_down)){
       return false;
     }
     
  }
  
  return true;
  

};


bool A_star::Extention_checks(Grid& grid, std::vector<int> &nodes){

  for(int i=0;i<nodes.size();i++){
     if(!Extention_check(grid, nodes[i])){
        return false
     }
  }

  return true;

};

bool A_star::Extention_check(Grid& grid, int current_node){

  int parent = grid.vertices_total[current_node].parent;

  if(parent==-1){
    return true;
  }

  if(parent>0 and parent<grid.vertices_total.size()-1){

    if(grid.vertices_total[current_node].metal==grid.vertices_total[parent].metal){
      return true;
    }else{

       int node_same_layer = trace_back_node(parent,grid);
       int metal = grid.vertices_total[parent].metal;
       int length = abs(grid.vertices_total[parent].x - grid.vertices_total[node_same_layer].x) + abs(grid.vertices_total[parent].y - grid.vertices_total[node_same_layer].y);
       int minL = drc_info.Metal_info[metal].minL;
       int delta_length = length - minL;

       if(delta_length<0){
           bool feasible = CheckExendable_With_Certain_Length(node_same_layer,parent,length,minL,grid);
           return feasible;
       }else{
           return true;
       }

    }
 
  }else{
    std::cout<<"Extention check bug parent node is out of grid"<<std::endl;
    assert(0);
  }

};



std::vector<std::vector<int> > A_star::A_star_algorithm(Grid& grid, int left_up, int right_down){

  int via_expand_effort = 100;

  std::set<std::pair<int,int>, RouterDB::pairComp> L_list;
  std::pair<int,int> temp_pair; 

  std::set<int> src_index;
  for(int i=0;i<(int)source.size();i++){
    
      src_index.insert(source[i]);

     }
  
  std::set<int> dest_index;
  for(int i=0;i<(int)dest.size();i++){
    
      dest_index.insert(dest[i]);

     }

  initial_source(grid, L_list, src_index);

  bool found = 0;
  int current_node = -1;

  //std::cout<<"A start checkout point2"<<std::endl;
  
  while(!L_list.empty() and !found){
    //std::cout<<"L_list size"<<L_list.size()<<std::endl;
    std::set<std::pair<int,int>, RouterDB::pairComp>::iterator it;
    it = L_list.begin();
    current_node = it->second;
    L_list.erase(it);
    
    //judge whether dest found Q2// judge whether dest works
    if(dest_index.find(current_node)!=dest_index.end()){
       found=1;
       continue;
      }

    //found the candidates nodes
    std::vector<int> candidate_node;
    //std::cout<<"A start checkout point3"<<std::endl;
    //std::cout<<"check point near node 1"<<std::endl;
    bool near_node_exist =found_near_node(current_node, grid, candidate_node);
    //std::cout<<"check point near node 2"<<std::endl;
    if(!near_node_exist){
       continue;
      }

    std::vector<int> temp_candidate_node;
    for(int i=0;i<candidate_node.size();i++){
       bool parallel = parallel_routing(grid, current_node, candidate_node[i], left_up, right_down, src_index, dest_index);
       if(parallel){
         temp_candidate_node.push_back(candidate_node[i]);
       }
    }

    candidate_node = temp_candidate_node;
    
    if(candidate_node==0){
       continue;
      }

    //std::cout<<"A start checkout point3.1"<<std::endl;

    //for each node judge whether can be expand or not; Q3 expandable?
    //bool expandable = 0;
    //std::vector<int> expand_candidate_node;
    for(int i=0;i<(int)candidate_node.size();i++){
       //judge whether this node can be expanded or not
       // not all node can be expanded?
       // if()?
       //std::cout<<"A start checkout point3.2"<<std::endl;
       int M_dis = Manhattan_distan(candidate_node[i], grid);
       //std::cout<<"A start checkout point3.3"<<std::endl;
       grid.vertices_total[candidate_node[i]].Cost = grid.vertices_total[current_node].Cost + abs(grid.vertices_total[current_node].x - grid.vertices_total[candidate_node[i]].x) + abs(grid.vertices_total[current_node].y - grid.vertices_total[candidate_node[i]].y) + via_expand_effort*abs(grid.vertices_total[candidate_node[i]].metal-grid.vertices_total[current_node].metal);
       int dis = grid.vertices_total[candidate_node[i]].Cost + M_dis;
       grid.vertices_total[candidate_node[i]].parent = current_node;
       temp_pair.first = dis;
       temp_pair.second = candidate_node[i];
       L_list.insert(temp_pair);
       }

  }
  //std::cout<<"A start checkout point4"<<std::endl;
  std::vector<std::vector<int> > temp_path; //Q4 return sheilding and parallel path?  sheild and parallel should be recovered in outer loop???
  if(found==0){
     std::cout<<"A_star fails to find a feasible path"<<std::endl;
    }else{
     temp_path = Trace_Back_Paths(grid, current_node, left, right, src_index, dest_index);
    }
   //std::cout<<"A start checkout point5"<<std::endl;
   refreshGrid(grid);


   return temp_path;
    
};

std::vector<std::vector<int> > A_star::Trace_Back_Paths(Grid& grid, int current_node, int left, int right, std::set<int> &src_index, std::set<int> &dest_index){

  std::vector<std::vector<int> > temp_paths;
  int mode = 0;
  std::vector<int> nodes;
  bool found = find_succsive_parallel_node(grid, current_node, left, right, mode, nodes, src_index, dest_index);
  if(!found){
    std::cout<<"Trace_Back_Paths bug 1 "<<std::endl;
    assert(0);
  }
  for(int i=0;i<nodes.size();i++){
     std::vector<int> temp_path = Trace_Back_Path(grid, nodes[i]);
     if(temp_path.size()<2){
        std::cout<<"Trace_Back_Paths bug 2 "<<std::endl;
        assert(0);      
     }
     temp_paths.push_back(temp_path);
  }

  if(shielding){
    if(temp_paths.size()>0){
      int path_size = temp_paths.size()-1;
      std::vector<int> temp_path_l = CovertToShieldingNet(grid, temp_paths[0]);
      temp_paths[0] = temp_path_l;
      std::vector<int> temp_path_r = CovertToShieldingNet(grid, temp_paths[path_size]);
      temp_path[path_size] = temp_path_r;
    }
  }
  
  return temp_paths;

};

std::vector<int> A_star::Trace_Back_Path(Grid& grid, int current_node){

  std::vector<int> temp_path;
  temp_path.push_back(current_node);
  int temp_parent = grid.vertices_total[current_node].parent;
  while(temp_parent!=-1){
      temp_path.push_back(temp_parent);
      temp_parent = grid.vertices_total[temp_parent].parent;
      }

  std::vector<int> reserse_path;
  for(int i=(int)temp_path.size()-1;i>=0;i--){
     reserse_path.push_back(temp_path[i]);
    }
  return reserse_path;
  

};


void A_star::CovertToShieldingNet(Grid& grid, std::vector<int> &temp_path){

  std::cout<<"start shielding path "<<"temp_path number "<<temp_path.size()<<std::endl;
  
  std::vector<int> temp_shielding_path;
  
  for(int i=1;i<(int)temp_path.size()-1;i++){

       temp_shielding_path.push_back(temp_path[i]);

     }

  //missing L shape remove

  std::cout<<"temp shielding number "<<temp_shielding_path.size()<<std::endl;

  return temp_shielding_path;

};

void A_star::refreshGrid(Grid& grid){

  for(int i=0;i<(int)grid.vertices_total.size();i++){
       grid.vertices_total[i].Cost = -1;
       grid.vertices_total[i].parent = -1;
     }
};

std::vector<std::vector<int>> A_star::GetPath(){
  std::vector<std::vector<int>> path(Path);
  return (path);
}
