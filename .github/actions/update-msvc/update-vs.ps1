for($count = 1; $true; $count++)
{
    $process = Start-Process -PassThru -Wait -NoNewWindow -FilePath  "C:\Program Files (x86)\Microsoft Visual Studio\Installer\setup.exe" -ArgumentList "update --quiet --norestart --force --channelId VisualStudio.17.Release --installpath ""C:\Program Files\Microsoft Visual Studio\2022\Enterprise""";

    $exit_code = $process.ExitCode;

    if($exit_code -eq 0) { break }

    echo "exited with $exit_code, tries count: $count. Retrying..."
}
