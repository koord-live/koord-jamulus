#requires -version 3

<#
.SYNOPSIS
    Get links to Microsoft Store downloads and optionally download packages

.DESCRIPTION
    Downloaded packages can be installed via Add-AppxPackage -Path

.PARAMETER packageFamilyName
    A list of package family names to download

.PARAMETER name
    A regular expression which will select the family package names for existing AppX package names that match

.PARAMETER packageTypeFilter
    The types of package to download

.PARAMETER downloadFolder
    The folder to download non-excluded files to. Will be created if does not exist. If not specified, files will not be downloaded

.PARAMETER ring
    The release ring to download

.PARAMETER proxy
    Proxy server to use

.PARAMETER excludeExtensions
    List of file extensions to not download

.PARAMETER excluderegex
    If download links match this regular expression they will not be downloaded

.PARAMETER language
    The language for downloading files

.PARAMETER force
    Overwrite files which already exist

.PARAMETER all
    Interrogate all local AppX packages rather than just those available to the user running the script (requires elevation). Use with -name

.EXAMPLE
    & '.\Get Store Downloads.ps1' -verbose -packageFamilyName Microsoft.MsixPackagingTool_8wekyb3d8bbwe -downloadFolder C:\@guyrleech\store-apps -excludeRegex '_arm__|_x86__|_arm64__' -proxy http://10.1.1.4:3128

    Download the resources from the Microsoft Store for the MSIX packaging tool, excluding files of arm, arm64 and x86 architecture, via the specified proxy and save to the folder C:\@guyrleech\store-apps

.NOTES
    Modification History:

    2022/03/18  @guyrleech  Initial public release
#>


<#
Copyright © 2022 Guy Leech

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, 
including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#>

[CmdletBinding()]

Param
(
    [Parameter(Mandatory=$true,ParameterSetName='Family')]
    [string[]]$packageFamilyName ,

    [Parameter(Mandatory=$true,ParameterSetName='Name')]
    [string]$name ,

    [Parameter(Mandatory=$false,ParameterSetName='Name')]
    [string]$packageTypeFilter = 'All' ,

    [string]$downloadFolder ,

    [ValidateSet('Slow','Retail','Fast,','RP')]
    [string]$ring = 'retail' ,

    [string]$proxy ,

    [string[]]$excludeExtensions = 'blockmap' ,

    [string]$excludeRegex ,

    [string]$language = 'en-GB' ,

    [switch]$force ,
    
    [Parameter(Mandatory=$false,ParameterSetName='Name')]
    [switch]$all
)

[hashtable]$proxyParameters = @{}

if( ! [string]::IsNullOrEmpty( $proxy ) )
{
      $proxyServer = New-Object -TypeName 'System.Net.WebProxy'
      $proxyServer.Address = [uri]$proxy
      $proxyServer.UseDefaultCredentials = $true
      [System.Net.WebRequest]::DefaultWebProxy = $proxyServer
      $proxyParameters.Add( 'Proxy' , $proxy )
      $proxyParameters.Add( 'ProxyUseDefaultCredentials' , $true )
      Write-Verbose "Proxy set to $proxy"
}

##https://stackoverflow.com/questions/41897114/unexpected-error-occurred-running-a-simple-unauthorized-rest-query?rq=1
Add-Type -TypeDefinition @'
public class SSLHandler
{
    public static System.Net.Security.RemoteCertificateValidationCallback GetSSLHandler()
    {
        return new System.Net.Security.RemoteCertificateValidationCallback((sender, certificate, chain, policyErrors) => { return true; });
    }
}
'@

[System.Net.ServicePointManager]::ServerCertificateValidationCallback = [SSLHandler]::GetSSLHandler()
[Net.ServicePointManager]::SecurityProtocol = [Net.ServicePointManager]::SecurityProtocol -bor [System.Net.SecurityProtocolType]::Tls12

if( $PSCmdlet.ParameterSetName -eq 'Name' )
{
    Import-Module -Name AppX -Verbose:$false
    [array]$packages = @( Get-AppxPackage -AllUsers:$all -PackageTypeFilter $PackageTypeFilter | Where-Object PackageFamilyName -Match $name )

    if( $null -eq $packages -or $packages.Count -eq 0 )
    {
        Throw "No existing AppX packages found matching $name"
    }
    $packageFamilyName = $packages | Select-Object -ExpandProperty PackageFamilyName -Unique
}

Write-Verbose -Message "Got $($packageFamilyName.Count) packages"

$webclient = $null

if( -Not [string]::IsNullOrEmpty( $downloadFolder ) )
{
    if( -Not ( Test-Path -Path $downloadFolder -ErrorAction SilentlyContinue ) )
    {
        $null = New-Item -Path $downloadFolder -Force -ItemType Directory -ErrorAction Stop
    }

    if( -Not ( $webClient = New-Object -TypeName System.Net.WebClient ) )
    {
        Throw "Failed to create a System.Net.WebClient object"
    }
}

[int]$count = 0

ForEach( $package in $packageFamilyName )
{
    $count++
    [string]$requestBody = "type=PackageFamilyName&url=$package&ring=$ring&lang=$language"
    $response = $null
    $session = $null
    $response = Invoke-WebRequest -URI 'https://store.rg-adguard.net/api/GetFiles' -body $requestBody -Method Post -ContentType 'application/x-www-form-urlencoded' -SessionVariable session @proxyParameters
    if( $response -and $response.PSObject.Properties[ 'links' ] -and $response.Links.Count -gt 0 )
    {
        ##Write-Verbose -Message "$count / $($packageFamilyName.Count) : Got $($response.Links.Count) links for $requestBody"
        ForEach( $link in ($response.Links | Where-Object href -match '^https?://' ))
        {
            [string]$extension = [System.IO.Path]::GetExtension( $link.innerText ) -replace '^\.'
            if( $extension -in $excludeExtensions )
            {
                Write-Verbose -Message "Ignoring `"$($link.innerText)`" as extension $extension in exclude list"
            }
            elseif( -Not [string]::IsNullOrEmpty( $excludeRegex ) -and $link.innerText -match $excludeRegex )
            {
                Write-Verbose -Message "Ignoring `"$($link.innerText)`" as matches $excludeRegex"
            }
            else
            {
                $result = [pscustomobject]@{
                        'PackageFamilyName' = $package
                        'File' = $link.innertext
                        'Link' = $link.href
                    }
                if( -Not [string]::IsNullOrEmpty( $downloadFolder )  )
                {
                    [string]$destinationFile = Join-Path -Path $downloadFolder -ChildPath $link.InnerText
                    if( ( Test-Path -Path $destinationFile -ErrorAction SilentlyContinue) -and -Not $force )
                    {
                        Write-Warning -Message "Not downloading to `"$destinationFile`" as already exists - use -force to overwrite"
                    }
                    else
                    {
                        try
                        {
                            $webclient.DownloadFile( ( $link.href -replace '&amp;' , '&' ) , $destinationFile )
                            if( $properties = Get-ItemProperty -Path $destinationFile )
                            {
                                Add-Member -InputObject $result -NotePropertyMembers @{
                                    'Download' = $properties.FullName
                                    'Size (KB)' = [math]::Round( $properties.Length / 1KB , 1 )
                                }
                            }
                            else
                            {
                                Write-Warning -Message "Unable to get file properties for $destinationFile"
                            }
                        }
                        catch
                        {
                            Write-Warning -Message "Problem downloading $($link.href) - $_"
                        }
                    }
                }
                $result
            }
        }
    }
    else
    {
        ## <img src="../img/stop.png">The server returned an empty list.<br>Either you have not entered the link correctly, or this service does not support generation for this product.</p><script type="text/javascript">
        Write-Warning -Message "No data or links returned for $requestBody - $($response.Content -split "`n`r?" , 2 -replace '\<[^\>]+\>' | Select-Object -First 1)"
    }
}
