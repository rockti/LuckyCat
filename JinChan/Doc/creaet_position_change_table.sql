CREATE TABLE `t_oper_positionchange` (
	`id` INT(11) NOT NULL AUTO_INCREMENT,
	`BrokerID` VARCHAR(11) NOT NULL DEFAULT '0',
	`InvestorID` VARCHAR(11) NOT NULL DEFAULT '0',
	`ExchangeID` VARCHAR(9) NOT NULL DEFAULT '0',
	`TradingDay` VARCHAR(9) NOT NULL DEFAULT '0',
	`InstrumentID` VARCHAR(11) NOT NULL DEFAULT '0',
	`ProductClass` CHAR(1) NOT NULL,
	`Direction` CHAR(1) NOT NULL,
	`OffsetFlag` CHAR(1) NOT NULL,
	`ChangeType` CHAR(1) NOT NULL,
	`Volume` INT(11) NOT NULL DEFAULT 0,
	`OrderLocalID` INT(11) NULL DEFAULT 0,
	`TradeID` VARCHAR(21) NULL DEFAULT '0',
	`OrderSysID` VARCHAR(21) NULL DEFAULT '0',
	`UserOrderLocalID` VARCHAR(21) NULL DEFAULT '0',
	`ExchangeSequenceNo` INT(11) NOT NULL DEFAULT 0,
	`PositionSequenceNo` INT(11) NOT NULL DEFAULT 0,
	`NotifyTime` VARCHAR(11) NULL DEFAULT NULL,
	PRIMARY KEY (`BrokerID`, `InvestorID`, `PositionSequenceNo`, `InstrumentID`, `TradingDay`),
	INDEX `id` (`id`)
)
COLLATE='utf8_general_ci'
ENGINE=InnoDB
AUTO_INCREMENT=1
;
