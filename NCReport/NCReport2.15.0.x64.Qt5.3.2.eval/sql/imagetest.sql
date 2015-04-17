CREATE TABLE IF NOT EXISTS `imagetest` (
  `img_ts` datetime NOT NULL default '0000-00-00 00:00:00',
  `filename` varchar(100) NOT NULL default '',
  `id` int(10) unsigned NOT NULL auto_increment,
  PRIMARY KEY  (`id`)
);


INSERT INTO `imagetest` (`img_ts`, `filename`, `id`) VALUES
('2008-01-17 07:47:00', '../images/joystick.png', 3),
('2008-03-10 06:30:00', '../images/printer.png', 2),
('2008-04-11 10:11:00', '../images/system.png', 1);
