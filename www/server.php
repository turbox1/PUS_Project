<?php

ob_start("ob_tidyhandler");

	header("HTTP/1.0 200 OK");
	header('Content-type: application/json; charset=utf-8');
	header("Cache-Control: no-cache, must-revalidate");
	header("Expires: Mon, 26 Jul 1997 05:00:00 GMT");
	header("Pragma: no-cache");

$i = (int)$_GET['i'];

$r = array('
{
    "success":true,
    "data":[
        {"class":"10", "type":"6", "id":"22", "params": {"unit":"0", "value":"12"}},
        {"class":"10", "type":"35", "id":"3", "params": {"value":"23"}},
        {"class":"10", "type":"6",  "id":"2", "params": {"value":"11"}},
        {"class":"10", "type":"35", "id":"4", "params": {"value":"41"}}
    ]
}',
'{
    "success":true,
    "data":[
        {"class":"10", "type":"6", "id":"22", "params": {"unit":"0", "value":"52"}},
        {"class":"10", "type":"35", "id":"3", "params": {"value":"23"}},
        {"class":"10", "type":"6",  "id":"2", "params": {"value":"41"}}
    ]
}',
'{
    "success":true,
    "data":[
        {"class":"10", "type":"6", "id":"22", "params": {"unit":"0", "value":"72"}},
        {"class":"10", "type":"35", "id":"3", "params": {"value":"23"}},
        {"class":"10", "type":"6",  "id":"2", "params": {"value":"21"}},
        {"class":"10", "type":"35", "id":"4", "params": {"value":"41"}}
    ]
}'
);

echo $r[$i];
	die();
?>
