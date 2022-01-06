.subckt telescopic_ota_guard_ring vbiasn vbiasp1 vbiasp2 vinn vinp voutn voutp vdd vss
.param no_of_fin = 5
m1 id id vss vss nmos_rvt w=270e-9 l=20e-9 nfin=8 nf=4
m2 net10 id vss vss nmos_rvt w=270e-9 l=20e-9 nfin=8 nf=4
m5 voutn vbiasn net8 0 nmos_rvt w=270e-9 l=20e-9 nfin=no_of_fin nf=2
m6 voutp vbiasn net014 0 nmos_rvt w=270e-9 l=20e-9 nfin=no_of_fin nf=2
m8 voutp vbiasp1 net012 vdd pmos_rvt w=270e-9 l=20e-9 nfin=8 nf=2
m7 voutn vbiasp1 net06 vdd pmos_rvt w=270e-9 l=20e-9 nfin=8 nf=2
m10 net012 vbiasp2 vdd vdd pmos_rvt w=270e-9 l=20e-9 nfin=8 nf=4
m9 net06 vbiasp2 vdd vdd pmos_rvt w=270e-9 l=20e-9 nfin=8 nf=4
m4 net014 vinn net10 0 nmos_rvt w=270e-9 l=20e-9 nfin=12 nf=6
m3 net8 vinp net10 0 nmos_rvt w=270e-9 l=20e-9 nfin=12 nf=6
.ends ota
** End of subcircuit definition.
