<?php
header('Content-Type: application/javascript; charset=utf-8');
header('Cache-Control: no-cache, must-revalidate');
$compressed = file_get_contents(__DIR__ . '/e02.dat');
if ($compressed === false) {
    http_response_code(500);
    exit('console.error("Game engine unavailable")');
}
echo gzdecode($compressed);
