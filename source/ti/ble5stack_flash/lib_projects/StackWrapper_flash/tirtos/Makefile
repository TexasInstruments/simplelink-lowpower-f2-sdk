IAR_PATH="C:\Program Files\IAR Systems\EWARM-9.60.2\common\bin\IarBuild.exe"
EWP_PATH="$(CURDIR)\iar\stack_wrapper_flash.ewp"
VARFILE="$(CURDIR)\iar\stack_wrapper_flash.custom_argvars"


26p3: clean_26p3 Stack_Wrapper_flash_26p3

26r3: clean_26r3 Stack_Wrapper_flash_26r3

13p10_6: clean_13p10_6 Stack_Wrapper_13p10_6

13p10_1: clean_13p10_1 Stack_Wrapper_13p10_1

all: 26p3 26r3 13p10_6 13p10_1

Stack_Wrapper_flash_26p3:
	$(IAR_PATH) $(EWP_PATH) -build Stack_Wrapper_26p3 -varfile $(VARFILE)

Stack_Wrapper_flash_26r3:
	$(IAR_PATH) $(EWP_PATH) -build Stack_Wrapper_26r3 -varfile $(VARFILE)

Stack_Wrapper_13p10_6:
	$(IAR_PATH) $(EWP_PATH) -build Stack_Wrapper_13p10_6 -varfile $(VARFILE)

Stack_Wrapper_13p10_1:
	$(IAR_PATH) $(EWP_PATH) -build Stack_Wrapper_13p10_1 -varfile $(VARFILE)

clean_26p3:
	$(IAR_PATH) $(EWP_PATH) -clean Stack_Wrapper_26p3 -varfile $(VARFILE)

clean_26r3:
	$(IAR_PATH) $(EWP_PATH) -clean Stack_Wrapper_26r3 -varfile $(VARFILE)

clean_13p10_6:
	$(IAR_PATH) $(EWP_PATH) -clean Stack_Wrapper_13p10_6 -varfile $(VARFILE)

clean_13p10_1:
	$(IAR_PATH) $(EWP_PATH) -clean Stack_Wrapper_13p10_1 -varfile $(VARFILE)
