#rem pg_dump --encoding=utf8 "host=192.168.75.10 port=5432 dbname=u7_compiler_tests_v383 user=u7 password=P2ssw0rd" > f:\\u7_compiler_tests_v383a.sql"
#rem pg_dump --encoding=utf8 "host=192.168.75.10 port=5432 dbname=u7_test_simulator_v383 user=u7 password=P2ssw0rd" > f:\\u7_test_simulator_v383.sql"


#rem pg_dump -h 192.168.75.10 -U u7 --encoding=utf8 -f f:\\u7_compiler_tests_v383a.sql u7_compiler_tests_v383
#SET PGPASSWORD=P2ssw0rd
#pg_dump -h 192.168.75.10 -U u7 -f f:\\u7_compiler_tests_v383.sql.1 u7_compiler_tests_v383

if ($args.count -ne 2)
{
  echo "Format: update_if_changed <SourceFile> <Target File>"
  exit;
}

echo n | comp.exe $args[0] $args[1] | Out-Null

if ($LASTEXITCODE -eq 1)
{
  Remove-Item -Force $args[1] 
  Rename-Item -Path $args[0] -NewName $args[1]
}
#rem pg_dump -h 192.168.75.10 -U u7 -f f:\\u7_test_simulator_v383.sql.1 u7_test_simulator_v383
#rem if ($LASTEXITCODE -eq 1){Remove-Item -Force \"f:\\TestDatabase\\u7_${Env:COMPILER_TESTS_PROJECT_NAME}.sql\"; Rename-Item -Path \"f:\\TestDatabase\\u7_${Env:COMPILER_TESTS_PROJECT_NAME}.sql.1\" -NewName \"f:\\TestDatabase\\u7_${Env:COMPILER_TESTS_PROJECT_NAME}.sql\";}



