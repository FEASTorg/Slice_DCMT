#!/usr/bin/make
KIBOT?=kibot
DEBUG?=
OUT_DIR=Generated
EXTRA_OPS=--banner -1 --log $(OUT_DIR)/kibot.log $(DEBUG)

SCHEM=BREAD_Slice.kicad_sch
BOARD=BREAD_Slice.kicad_pcb
EXTRA_OPS += -e $(SCHEM) -b $(BOARD)

.PHONY: erc drc sch_fab pcb_fab drc_and_fab

erc:
	$(KIBOT) $(EXTRA_OPS) -d $(OUT_DIR) -s erc -i

drc:
	$(KIBOT) $(EXTRA_OPS) -d $(OUT_DIR) -s drc -i

sch_fab:
	$(KIBOT) $(EXTRA_OPS) -d $(OUT_DIR) -s erc,drc,print_sch,interactive_bom,bom_html,bom_csv

pcb_fab:
	$(KIBOT) $(EXTRA_OPS) -d $(OUT_DIR) -s print_pcb,gerbers,excellon_drill,gerber_drills,position,board_top_png,board_bottom_png

drc_and_fab:
	$(KIBOT) $(EXTRA_OPS) -d $(OUT_DIR) -s drc,print_pcb,gerbers,excellon_drill,gerber_drills,position
