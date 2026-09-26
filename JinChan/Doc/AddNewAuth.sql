ALTER TABLE `t_sync_investorinfo`
	ADD COLUMN `IsNewAuth` CHAR(1) NOT NULL DEFAULT '0' AFTER `ClientID`;
ALTER TABLE `t_sync_investorinfo`
	ADD COLUMN `Salt` VARCHAR(32) NULL DEFAULT NULL AFTER `IsNewAuth`;
ALTER TABLE `t_sync_investorinfo`
	ADD COLUMN `PasswordHash` VARCHAR(128) NULL DEFAULT NULL AFTER `Salt`;
UPDATE `t_sync_investorinfo` SET Salt=LEFT(UUID(),8);