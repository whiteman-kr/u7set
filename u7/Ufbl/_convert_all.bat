for %%F in (*.templ_ufb) do (
   file2pgsql.exe %%F %%F.sql "$root$/UFBL"
)

del _ufbl_all.sql
copy *.sql _ufbl_all.sql
del *.templ_ufb.sql