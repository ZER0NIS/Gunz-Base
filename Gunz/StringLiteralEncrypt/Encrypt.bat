REM cxr파일들을 헤더파일로 인코딩하는 배치파일입니다.
REM 이 배치파일은 빌드전 이벤트로 실행되도록 합니다.
REM 인코딩된 파일은 이름에 cxr_ 접두어를 붙이기로 합니다.
REM 대상파일들이 모두 동일한 패스워드를 사용해야 한다는점을 주의하세요. (디코더를 공유하기 때문에)
REM .

SET CXRPASSWORD=eJh^5ihjB*)uQB(#]($1;vx

..\Gunz\StringLiteralEncrypt\cxr -i	test.cxr		-o cxr_test.h			-p %CXRPASSWORD%
..\Gunz\StringLiteralEncrypt\cxr -i MMatchItem.cxr	-o cxr_MMatchItem.h		-p %CXRPASSWORD%
..\Gunz\StringLiteralEncrypt\cxr -i MMatchBuff.cxr	-o cxr_MMatchBuff.h		-p %CXRPASSWORD%
