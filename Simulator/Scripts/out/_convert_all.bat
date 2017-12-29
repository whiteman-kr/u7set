for %%F in (*.js) do (
   file2pgsql.exe %%F %%F.sql "$root$/MC"
)

rem del _afbl_all.sql
rem copy *.sql _afbl_all.sql