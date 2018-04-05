for %%F in (*.js) do (
   file2pgsql.exe %%F %%F.sql "$root$/MC"
)

