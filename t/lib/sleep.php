<?php

function nginx_sleep(){
	yield ngx::sleep(1);
}

function nginx_msleep(){
	yield ngx_msleep(1);
}
