************************************************************************
* auCdl Netlist:
* 
* Library Name:  OTA_class
* Top Cell Name: single_ended_cascode_pmos
* View Name:     schematic
* Netlisted on:  Sep 11 21:07:21 2019
************************************************************************

*.BIPOLAR
*.RESI = 2000 
*.RESVAL
*.CAPVAL
*.DIOPERI
*.DIOAREA
*.EQUATION
*.SCALE METER
*.MEGA
.PARAM

*.GLOBAL gnd!
+        vdd!

*.PIN gnd!
*+    vdd!

************************************************************************
* Library Name: OTA_class
* Cell Name:    single_ended_cascode_pmos
* View Name:    schematic
************************************************************************

.SUBCKT single_ended_cascode_pmos Vbiasn1 Vbiasn2 Vbiasp1 Vbiasp2 Vinn Vinp 
+ Voutp
*.PININFO Vbiasp1:I Vbiasp2:I Vinn:I Vinp:I Vbiasn1:O Vbiasn2:O Voutp:O
MM4 net28 Vinn net12 net22 pmos_rvt w=WA l=LA nfin=nA
MM3 net29 Vinp net12 net22 pmos_rvt w=WA l=LA nfin=nA
MM0 net12 Vbiasp1 vdd! vdd! pmos_rvt w=WA l=LA nfin=nA
MM6 net16 Vbiasp2 net25 vdd! pmos_rvt w=WA l=LA nfin=nA
MM5 Voutp Vbiasp2 net24 vdd! pmos_rvt w=WA l=LA nfin=nA
MM2 net25 net16 vdd! vdd! pmos_rvt w=WA l=LA nfin=nA
MM1 net24 net16 vdd! vdd! pmos_rvt w=WA l=LA nfin=nA
MM10 net29 Vbiasn1 gnd! gnd! nmos_rvt w=WA l=LA nfin=nA
MM9 net28 Vbiasn1 gnd! gnd! nmos_rvt w=WA l=LA nfin=nA
MM8 net16 Vbiasn2 net29 gnd! nmos_rvt w=WA l=LA nfin=nA
MM7 Voutp Vbiasn2 net28 gnd! nmos_rvt w=WA l=LA nfin=nA
.ENDS


.SUBCKT LG_load_biasn_LV Vbiasn2 Biasp
*.PININFO Biasp:I Vbiasn2:O
MM13 net9 Vbiasn2 gnd! gnd! nmos_rvt w=WA l=LA nfin=nA
MM15 Vbiasn2 Vbiasn2 net9 gnd! nmos_rvt w=WA l=LA nfin=nA
MM14 Vbiasn2 Biasp vdd! vdd! pmos_rvt w=WA l=LA nfin=nA
.ENDS

.SUBCKT CR15_1 Vbiasp
*.PININFO Vbiasp:O
RR1 Vbiasp gnd! res=rK
MM2 Vbiasp Vbiasp vdd! vdd! pmos_rvt w=WA l=LA nfin=nA
.ENDS


xiota LG_Vbiasn1 LG_Vbiasn2 LG_Vbiasp1 LG_Vbiasp2 Vinn Vinp single_ended_cascode_pmos
xiLG_load_biasn_LV Biasp LG_Vbiasn2 LG_load_biasn_LV
xibCR15_1 Biasp CR15_1
.END