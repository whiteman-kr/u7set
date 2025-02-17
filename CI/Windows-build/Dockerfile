FROM mcr.microsoft.com/windows/servercore:ltsc2022

# ----------------------------------------- Install Far -----------------------------------------

# Restore the default Windows shell for correct batch processing.
SHELL ["cmd", "/S", "/C"]

RUN \
    # Download the Build Tools bootstrapper.
    curl -SL --output vs_buildtools.exe https://aka.ms/vs/17/release/vs_buildtools.exe \
    \
    # Install Build Tools with the Microsoft.VisualStudio.Workload.AzureBuildTools workload, excluding workloads and components with known issues.
    && (start /w vs_buildtools.exe --quiet --wait --norestart --nocache \
        --installPath "%ProgramFiles(x86)%\Microsoft Visual Studio\2022\BuildTools" \
	--add Microsoft.VisualStudio.Workload.VCTools \
	--includeRecommended \
        || IF "%ERRORLEVEL%"=="3010" EXIT 0) \
    \
    # Cleanup
    && del /q vs_buildtools.exe

WORKDIR C:/INSTALL

COPY "INSTALL/Far.zip" .
RUN powershell -Command \
    $ErrorActionPreference = 'Stop'; \
    Expand-Archive -Path Far.zip -DestinationPath c:\ ; \
    Remove-Item Far.zip -Force

COPY "INSTALL/vld-2.5.1-setup.exe" .
RUN powershell -Command \
    $ErrorActionPreference = 'Stop'; \
    Start-Process vld-2.5.1-setup.exe -Wait -ArgumentList '/SILENT /SUPPRESSMSGBOXES /NORESTART' ; \
    Remove-Item vld-2.5.1-setup.exe -Force

COPY "INSTALL/git.zip" .
RUN powershell -Command \
    $ErrorActionPreference = 'Stop'; \
    Expand-Archive -Path git.zip -DestinationPath c:\ ; \
    Remove-Item git.zip -Force

COPY "INSTALL/6.5.3.zip" .
RUN powershell -Command \
    $ErrorActionPreference = 'Stop'; \
    Expand-Archive -Path 6.5.3.zip -DestinationPath c:\Qt ; \
    Remove-Item 6.5.3.zip -Force

COPY "INSTALL/doxygen-1.11.0.windows.x64.bin.zip" .
RUN powershell -Command \
    $ErrorActionPreference = 'Stop'; \
    Expand-Archive -Path doxygen-1.11.0.windows.x64.bin.zip -DestinationPath c:\Doxygen ; \
    Remove-Item doxygen-1.11.0.windows.x64.bin.zip -Force

COPY "INSTALL/windows_10_cmake_Release_Graphviz-11.0.0-win64.zip" .
RUN powershell -Command \
    $ErrorActionPreference = 'Stop'; \
    Expand-Archive -Path windows_10_cmake_Release_Graphviz-11.0.0-win64.zip -DestinationPath c:\Graphviz ; \
    Remove-Item windows_10_cmake_Release_Graphviz-11.0.0-win64.zip -Force

COPY "INSTALL/TortoiseSVN.zip" .
RUN powershell -Command \
    $ErrorActionPreference = 'Stop'; \
    Expand-Archive -Path TortoiseSVN.zip -DestinationPath c:\ ; \
    Remove-Item TortoiseSVN.zip -Force

COPY "INSTALL/ROVT.zip" .
RUN powershell -Command \
    $ErrorActionPreference = 'Stop'; \
    Expand-Archive -Path ROVT.zip -DestinationPath c:\ ; \
    Remove-Item ROVT.zip -Force

COPY "INSTALL/radiy-code-signing.zip" .
RUN powershell -Command \
    $ErrorActionPreference = 'Stop'; \
    Expand-Archive -Path radiy-code-signing.zip -DestinationPath c:\ ; \
    Remove-Item radiy-code-signing.zip -Force

RUN echo 192.168.14.242 delo >> "C:\Windows\System32\drivers\etc\hosts"

WORKDIR C:/ftdi
COPY "INSTALL/CDM-v2.12.36.4-WHQL-Certified.zip" .
COPY "INSTALL/ftd2xx.dll" .

RUN setx /M PATH "C:\Qt\6.5.3\msvc2019_64\bin;%PATH%"
RUN setx /M PATH "C:\Qt\Tools\QtCreator\bin\jom;%PATH%"
RUN setx /M PATH "C:\Qt\Tools\QtInstallerFramework\4.5\bin;%PATH%"
RUN setx /M PATH "C:\Doxygen;%PATH%"
RUN setx /M PATH "C:\graphviz\Graphviz-11.0.0-win64\bin;%PATH%"
RUN setx /M PATH "C:\TortoiseSVN\bin;%PATH%"
RUN setx /M PATH "C:\git\bin;%PATH%"
RUN setx /M SIGN_PASSWORD "Mkp1DPcfsTKI"

RUN powershell -Command \
    Start-Process git -Wait -ArgumentList 'config --global --add safe.directory C:/builds/boris.vintenko/u7setdocker'

COPY "INSTALL/winsdksetup.exe" .
RUN winsdksetup.exe /features OptionId.SigningTools /quiet /norestart && del /q winsdksetup.exe

# WORKDIR C:/Gitlab-Runner
# COPY "INSTALL/gitlab-runner-windows-amd64.exe" . 

# RUN C:\\Gitlab-Runner\\gitlab-runner-windows-amd64.exe install

WORKDIR C:/
COPY "INSTALL/autoexec.bat" . 

ENTRYPOINT ["c:\\autoexec.bat", "&&", "powershell.exe", "-NoLogo", "-ExecutionPolicy", "Bypass"]
#CMD [ "cmd.exe" ]
