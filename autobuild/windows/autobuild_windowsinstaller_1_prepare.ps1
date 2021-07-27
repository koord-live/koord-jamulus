# Powershell

# autobuild_1_prepare: set up environment, install Qt & dependencies


###################
###  PROCEDURE  ###
###################

echo "Install Qt..."
# Install Qt
#pip install aqtinstall
pip install aqtinstall==v2.0.0b5 #FIXME temp version to allow vcredist, tools install
echo "Get Qt 64 bit..."
# intermediate solution if the main server is down: append e.g. " -b https://mirrors.ocf.berkeley.edu/qt/" to the "aqt"-line below
aqt install --outputdir C:\Qt 5.15.2 windows desktop win64_msvc2019_64
# install tools - vcredist, cmake
aqt tool windows desktop --outputdir C:\Qt tools_vcredist qt.tools.vcredist_msvc2019_x64
aqt tool windows desktop --outputdir C:\Qt tools_cmake qt.tools.cmake.win64

# echo "Get Qt 32 bit..."
# # intermediate solution if the main server is down: append e.g. " -b https://mirrors.ocf.berkeley.edu/qt/" to the "aqt"-line below
# aqt install --outputdir C:\Qt 5.15.2 windows desktop win32_msvc2019

# echo "Get Qt WinRT 64 bit ..."
# # intermediate solution if the main server is down: append e.g. " -b https://mirrors.ocf.berkeley.edu/qt/" to the "aqt"-line below
# aqt install --outputdir C:\Qt 5.15.2 windows winrt win64_msvc2019_winrt_x64
