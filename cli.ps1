param(
    [parameter(Position=0,Mandatory=$true,ParameterSetName="Mount")][switch]$Mount,
    [parameter(Position=0,Mandatory=$true,ParameterSetName="Unmount")][switch]$Unmount,
    [parameter(Mandatory=$true,ParameterSetName="Mount")][string][ValidatePattern('^[0-9a-f]{8}-([0-9a-f]{4}-){3}[0-9a-f]{12}$')]$VolumeGUID,
    [parameter(Mandatory=$true)][string][ValidatePattern('^[A-Z]$')]$DriveLetter
);

[byte[]] $pipeData = @();
$encoder = [system.Text.Encoding]::ASCII;
If($Mount -eq $True) {
    $pipeData += 0;
    $pipeData += $encoder.GetBytes($DriveLetter);
    $pipeData += $encoder.GetBytes($VolumeGUID);
} Else {
    $pipeData += 1;
    $pipeData += $encoder.GetBytes($DriveLetter);
}

$pipe = new-object System.IO.Pipes.NamedPipeClientStream('.', 'VolumeMounterService',
    [System.IO.Pipes.PipeDirection]::InOut, [System.IO.Pipes.PipeOptions]::None, 
    [System.Security.Principal.TokenImpersonationLevel]::Impersonation);
$pipe.Connect(100);
$pipe.Write($pipeData, 0, $pipeData.Length);
$pipe.Dispose();
