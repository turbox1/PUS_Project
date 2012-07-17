<?php

ob_start("ob_tidyhandler");

	header("HTTP/1.0 200 OK");
	header('Content-type: application/json; charset=utf-8');
	header("Cache-Control: no-cache, must-revalidate");
	header("Expires: Mon, 26 Jul 1997 05:00:00 GMT");
	header("Pragma: no-cache");

//$i = (int)$_GET['i'];

echo '
[
        {"id":"1", "name":"Pierwsza lista", "tree": ""},
        {"id":"2", "name":"Druga lista", "tree": "'.addslashes('<ul><li rel="root" class="jstree-open" style=""><ins class="jstree-icon">&nbsp;</ins><a href="#" class="jstree-clicked"><ins class="jstree-icon">&nbsp;</ins>PPPPPPAAAA</a><ul><li id="tDevice-20-3-43" rel="default" class="jstree-leaf"><ins class="jstree-icon">&nbsp;</ins><a href="#" class=""><ins class="jstree-icon">&nbsp;</ins>DDD</a></li><li id="tDevice-10-6-22" rel="default" class="jstree-leaf"><ins class="jstree-icon">&nbsp;</ins><a href="#" class=""><ins class="jstree-icon">&nbsp;</ins>Termometr22</a></li><li id="tDevice-10-35-4" rel="default" class="jstree-leaf jstree-last"><ins class="jstree-icon">&nbsp;</ins><a href="#" class=""><ins class="jstree-icon">&nbsp;</ins>Hygrometr4</a></li></ul></li><li id="aDevices" rel="root" class="jstree-open jstree-last"><ins class="jstree-icon">&nbsp;</ins><a href="#" class=""><ins class="jstree-icon">&nbsp;</ins>Dostępne urządzenia</a><ul><li id="tDevice-20-3-43" rel="default" class="jstree-leaf"><ins class="jstree-icon">&nbsp;</ins><a href="#"><ins class="jstree-icon">&nbsp;</ins>Przycisk43</a></li><li id="tDevice-10-35-4" rel="default" class="jstree-leaf"><ins class="jstree-icon">&nbsp;</ins><a href="#"><ins class="jstree-icon">&nbsp;</ins>Hygrometr4</a></li><li id="tDevice-10-6-2" rel="default" class="jstree-leaf"><ins class="jstree-icon">&nbsp;</ins><a href="#"><ins class="jstree-icon">&nbsp;</ins>Termometr2</a></li><li id="tDevice-10-35-3" rel="default" class="jstree-leaf"><ins class="jstree-icon">&nbsp;</ins><a href="#"><ins class="jstree-icon">&nbsp;</ins>Hygrometr3</a></li><li id="tDevice-10-6-22" rel="default" class="jstree-leaf jstree-last"><ins class="jstree-icon">&nbsp;</ins><a href="#"><ins class="jstree-icon">&nbsp;</ins>Termometr22</a></li></ul></li></ul>').'"},
        {"id":"3", "name":"Trzecia lista", "tree": ""}
    ]

';

	die();
?>
