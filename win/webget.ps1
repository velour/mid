param($url, $dest)

$client = new-object System.Net.WebClient
$client.DownloadFile($url, $dest)
