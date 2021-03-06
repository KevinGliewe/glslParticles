
Add-Type -AssemblyName System.IO.Compression.FileSystem
function Unzip
{
    param([string]$zipfile, [string]$outpath)

    [System.IO.Compression.ZipFile]::ExtractToDirectory($zipfile, $outpath)
}

function Import-Lib {
    param ([string]$libdir, [string]$libname, [string]$liburl)

    $libzip = $libdir + "/" + $libname + ".zip"

    Write-Output "Downloading $libname from $liburl"
    Invoke-WebRequest -Uri $liburl -OutFile $libzip

    Write-Output "Unpacking $libzip"
    Unzip $libzip $libdir
    Remove-Item $libzip
}

[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12

$libsdir = $PSScriptRoot + "/../libs/"

Import-Lib $libsdir "glew" "https://datapacket.dl.sourceforge.net/project/glew/glew/2.1.0/glew-2.1.0-win32.zip"

Import-Lib $libsdir "glfw64" "https://github.com/glfw/glfw/releases/download/3.2.1/glfw-3.2.1.bin.WIN64.zip"
Import-Lib $libsdir "glfw32" "https://github.com/glfw/glfw/releases/download/3.2.1/glfw-3.2.1.bin.WIN32.zip"

Import-Lib $libsdir "glm" "https://github.com/g-truc/glm/releases/download/0.9.8.5/glm-0.9.8.5.zip"

$bin32dir = $PSScriptRoot + "/../src/GlslParticles/Win32/Debug/"
$bin64dir = $PSScriptRoot + "/../src/GlslParticles/x64/Debug/"

New-Item -Path $bin32dir -ItemType Directory
New-Item -Path $bin64dir -ItemType Directory

Copy-Item -Path "$libsdir/glew-2.1.0/bin/Release/Win32/glew32.dll" -Destination $bin32dir
Copy-Item -Path "$libsdir/glew-2.1.0/bin/Release/x64/glew32.dll" -Destination $bin64dir