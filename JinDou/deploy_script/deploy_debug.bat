del release\* /q /s
xcopy /E /Y dll\* debug\
xcopy /E /Y qt_debug\* debug\
encryptor -e config\jindou_dr.toml debug\jindou.sol
xcopy /e /y  config\style.white.qss debug\
copy exe\jindou_dr.exe debug\