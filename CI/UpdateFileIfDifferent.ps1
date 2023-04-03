if ($args.count -ne 2)
{
  echo "Format: update_if_changed <SourceFile> <Target File>"
  exit;
}

$fileSource = $args[0]
$fileTarget = $args[1]

comp.exe /M $fileSource $fileTarget

if ($LASTEXITCODE -eq 1) 
{
  echo "Updating file $fileSource -> $fileTarget"
  Remove-Item -Force $fileTarget;
  Rename-Item -Path $fileSource -NewName $fileTarget
}

$LASTEXITCODE = 0
