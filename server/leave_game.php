<?php 
include 'config.php';

$connection = mysql_connect ("localhost", $mysql_username, $mysql_password) or die;
mysql_select_db("sparrowman") or die;

$game_id = (int)$_POST['game_id'];
$player_id = (int)$_POST['player_id'];
$player_pw = (int)$_POST['player_pw'];

$query = "SELECT * FROM hase_player_list WHERE game_id = '$game_id' AND  player_id = '$player_id'";
$result = mysql_query($query) or die;

$row = mysql_fetch_assoc( $result );
if ($row['player_pw'] == $player_pw)
{
	$query = "SELECT * FROM hase_game_list WHERE game_id = '$game_id'";
	$result = mysql_query($query) or die;
	$row = mysql_fetch_array( $result );
	if ($row['status'] == 0) //Never started
	{
		$query = "DELETE FROM hase_player_list WHERE game_id = '$game_id' AND player_id = '$player_id'";
		mysql_query($query) or die;
	}
	else
	{
		$query = "UPDATE hase_player_list SET status='-1' WHERE game_id = '$game_id' AND player_id = '$player_id'";
		mysql_query($query) or die;
	}	
}
mysql_close($connection); 
?>