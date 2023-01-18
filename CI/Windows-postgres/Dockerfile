FROM mcr.microsoft.com/windows/servercore:ltsc2022

# ----------------------------------------- Install Far -----------------------------------------

# Restore the default Windows shell for correct batch processing.
SHELL ["cmd", "/S", "/C"]

WORKDIR C:/INSTALL

COPY "INSTALL/Far.zip" .
RUN powershell -Command \
    $ErrorActionPreference = 'Stop'; \
    Expand-Archive -Path Far.zip -DestinationPath c:\ ; \
    Remove-Item Far.zip -Force

COPY "INSTALL/git.zip" .
RUN powershell -Command \
    $ErrorActionPreference = 'Stop'; \
    Expand-Archive -Path git.zip -DestinationPath c:\ ; \
    Remove-Item git.zip -Force

COPY "INSTALL/postgresql-15.1-1-windows-x64.exe" .
RUN \
#   Install PostgreSQL
    postgresql-15.1-1-windows-x64.exe --mode unattended --unattendedmodeui none --superpassword P2ssw0rd\
#   Cleanup
    && del /q postgresql-15.1-1-windows-x64.exe

RUN setx /M PATH "C:\Program Files\PostgreSQL\15\bin;%PATH%"
RUN setx /M PATH "C:\git\bin;%PATH%"
RUN setx /M PGPASSWORD "P2ssw0rd"

RUN powershell -Command \
    Start-Process git -Wait -ArgumentList 'config --global --add safe.directory C:/builds/boris.vintenko/u7setdocker'

WORKDIR C:/
COPY "INSTALL/autoexec.bat" . 

ENTRYPOINT ["c:\\autoexec.bat", "&&", "powershell.exe", "-NoLogo", "-ExecutionPolicy", "Bypass"]
#CMD [ "cmd.exe " ]
