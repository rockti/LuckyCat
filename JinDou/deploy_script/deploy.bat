del release\* /q /s
xcopy /E /Y dll\* release\
xcopy /E /Y qt_release\* release\
encryptor -e config\jindou.toml release\jindou.sol
xcopy /e /y  config\style.white.qss release\
copy exe\jindou.exe release\