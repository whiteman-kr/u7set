for %%F in (*.xml) do (
   file2pgsql.exe %%F %%F.sql "$root$/AFBL"
)

rem del _afbl_all.sql
rem copy *.sql _afbl_all.sql