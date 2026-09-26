CREATE TABLE `t_oper_balancechange` (
	`id` INT(11) NOT NULL AUTO_INCREMENT,
	`BrokerID` VARCHAR(11) NOT NULL,
	`InvestorID` VARCHAR(11) NOT NULL,
	`TradingDay` VARCHAR(9) NOT NULL DEFAULT '',
	`BalanceChangeType` CHAR(10) NOT NULL,
	`Amount` DECIMAL(18,4) NOT NULL DEFAULT 0.0000,
	`ExchangeSequenceNo` INT(11) NOT NULL DEFAULT 0,
	`BalanceSequenceNo` INT(11) NOT NULL DEFAULT 0,
	`ChangeTime` VARCHAR(11) NULL DEFAULT NULL,
	PRIMARY KEY (`BrokerID`, `InvestorID`, `TradingDay`, `BalanceSequenceNo`),
	INDEX `id` (`id`)
)
COLLATE='utf8_general_ci'
ENGINE=InnoDB
AUTO_INCREMENT=1
;
