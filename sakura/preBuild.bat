@echo off

HeaderMake -in=..\sakura_core\Funccode_x.hsrc -out=..\sakura_core\Funccode_define.h -mode=define
HeaderMake -in=..\sakura_core\Funccode_x.hsrc -out=..\sakura_core\Funccode_enum.h -mode=enum -enum=EFunctionCode
MakefileMake -file=..\sakura_core\Makefile -dir=..\sakura_core

rem hg parent --template "#define HG_REV \"{latesttag}\"\n" > "..\sakura_core\hgrev.h"
rem hg parent --template "#define HG_REV_DISTANCE {latesttagdistance}\n" >> "..\sakura_core\hgrev.h"

for /F "usebackq delims=" %%A in (`git describe`) do set GITREV=%%A
echo #define GIT_REV "%GITREV%" > ..\sakura_core\gitrev.h

exit 0
