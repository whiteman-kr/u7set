for %%F in (*.afb) do (
   file2pgsql.exe %%F %%F.sql "$root$/AFBL"
)

del _afbl_all.sql
copy *.sql _afbl_all.sql