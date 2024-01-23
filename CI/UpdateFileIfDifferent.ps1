if ($args.count -ne 2)
{
  echo "Format: update_if_changed <SourceFile> <Target File>"
  exit;
}

$fileSource = $args[0]
$fileTarget = $args[1]

if (!(Test-Path $fileTarget)) 
{
  echo "Target file $$fileTarget does not exist, creating it from source."
  Rename-Item -Path $fileSource -NewName $fileTarget
}
else
{
  comp.exe /M $fileSource $fileTarget
	
  if ($LASTEXITCODE -eq 1) 
  {
    echo "Updating file $fileSource -> $fileTarget"
    Remove-Item -Force $fileTarget;
    Rename-Item -Path $fileSource -NewName $fileTarget
  }
}

$LASTEXITCODE = 0
