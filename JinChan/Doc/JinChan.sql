-- --------------------------------------------------------
-- 主机:                           127.0.0.1
-- 服务器版本:                        10.4.12-MariaDB - mariadb.org binary distribution
-- 服务器OS:                        Win64
-- HeidiSQL 版本:                  10.2.0.5599
-- --------------------------------------------------------

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!50503 SET NAMES utf8mb4 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;


-- Dumping database structure for jinchan
CREATE DATABASE IF NOT EXISTS `jinchan` /*!40100 DEFAULT CHARACTER SET utf8 */;
USE `jinchan`;

-- Dumping structure for table jinchan.t_oper_balancechange
CREATE TABLE IF NOT EXISTS `t_oper_balancechange` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `BrokerID` varchar(11) NOT NULL DEFAULT '0',
  `InvestorID` varchar(11) NOT NULL DEFAULT '0',
  `BalanceChangeType` char(1) NOT NULL DEFAULT '0',
  `Amount` double NOT NULL DEFAULT 0,
  `ExchangeSequenceNo` int(11) NOT NULL DEFAULT 0,
  `BalanceSequenceNo` int(11) NOT NULL DEFAULT 0,
  `ChangeTime` varchar(11) DEFAULT NULL,
  PRIMARY KEY (`BrokerID`,`InvestorID`,`BalanceSequenceNo`),
  KEY `id` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Data exporting was unselected.

-- Dumping structure for table jinchan.t_oper_order
CREATE TABLE IF NOT EXISTS `t_oper_order` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `BrokerID` varchar(11) DEFAULT NULL,
  `OrderOperatorID` varchar(11) NOT NULL,
  `InvestorID` varchar(11) NOT NULL,
  `ExchangeID` varchar(9) NOT NULL,
  `ProductClass` char(1) NOT NULL,
  `InstrumentID` varchar(11) NOT NULL,
  `Direction` char(1) NOT NULL,
  `OffsetFlag` char(1) NOT NULL,
  `ActionLocalID` int(11) NOT NULL DEFAULT 0,
  `LimitPrice` decimal(12,3) NOT NULL,
  `Volume` decimal(8,0) NOT NULL,
  `OrderPriceType` char(1) NOT NULL,
  `UserOrderLocalID` varchar(16) NOT NULL,
  `UserActionLocalID` varchar(16) NOT NULL,
  `OrderSysID` varchar(21) NOT NULL,
  `OrderStatus` char(1) NOT NULL,
  `TradingDay` varchar(9) NOT NULL,
  `SeatID` varchar(11) NOT NULL,
  `ClientID` varchar(13) NOT NULL,
  `InsertTime` varchar(9) NOT NULL,
  `InsertErrorCode` int(8) DEFAULT NULL,
  `CancelTime` varchar(9) DEFAULT NULL,
  `CancelOperatorID` varchar(11) DEFAULT NULL,
  `VolumeTrade` decimal(8,0) DEFAULT NULL,
  `VolumeRemain` decimal(8,0) NOT NULL,
  `OrderLocalID` int(11) NOT NULL DEFAULT 0,
  `FrozenMargin` double(16,4) DEFAULT NULL,
  `FrozenFee` double(16,4) DEFAULT NULL,
  `TradeFee` double(16,4) DEFAULT NULL,
  `CancelErrorCode` int(8) DEFAULT NULL,
  `TradeMargin` double(16,4) DEFAULT NULL,
  `UserCustom` varchar(32) DEFAULT NULL,
  `SequenceNo` int(11) DEFAULT NULL,
  PRIMARY KEY (`OrderLocalID`,`TradingDay`,`OrderSysID`),
  KEY `id` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=59 DEFAULT CHARSET=utf8;

-- Data exporting was unselected.

-- Dumping structure for table jinchan.t_oper_positionchange
CREATE TABLE IF NOT EXISTS `t_oper_positionchange` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `BrokerID` varchar(11) NOT NULL,
  `InvestorID` varchar(11) NOT NULL,
  `ExchangeID` varchar(9) NOT NULL,
  `InstrumentID` varchar(11) NOT NULL,
  `ProductClass` char(1) NOT NULL,
  `Direction` char(1) NOT NULL,
  `OffsetFlag` char(1) NOT NULL,
  `ChangeType` char(1) NOT NULL,
  `Volume` int(11) NOT NULL DEFAULT 0,
  `OrderLocalID` varchar(16) DEFAULT NULL,
  `TradeID` varchar(16) DEFAULT NULL,
  `OrderSysID` varchar(21) DEFAULT NULL,
  `UserOrderLocalID` varchar(16) DEFAULT NULL,
  `ExchangeSequenceNo` int(12) NOT NULL DEFAULT 0,
  `PositionSequenceNo` int(12) NOT NULL DEFAULT 0,
  `TradingDay` varchar(12) DEFAULT NULL,
  `NotifyTime` varchar(11) DEFAULT NULL,
  PRIMARY KEY (`BrokerID`,`InvestorID`,`InstrumentID`,`PositionSequenceNo`),
  KEY `id` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=56 DEFAULT CHARSET=utf8;

-- Data exporting was unselected.

-- Dumping structure for table jinchan.t_oper_trade
CREATE TABLE IF NOT EXISTS `t_oper_trade` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `BrokerID` varchar(11) DEFAULT NULL,
  `InvestorID` varchar(11) DEFAULT NULL,
  `ExchangeID` varchar(9) NOT NULL,
  `ProductClass` char(1) DEFAULT NULL,
  `InstrumentID` varchar(11) DEFAULT NULL,
  `Direction` char(1) DEFAULT NULL,
  `OffsetFlag` char(1) DEFAULT NULL,
  `TradeVolume` decimal(8,0) DEFAULT NULL,
  `TradePrice` decimal(12,3) DEFAULT NULL,
  `TradeAmt` double(18,4) DEFAULT NULL,
  `TradeDate` varchar(9) NOT NULL,
  `TradeTime` varchar(9) DEFAULT NULL,
  `TradeID` varchar(21) NOT NULL,
  `OrderSysID` varchar(21) NOT NULL,
  `UserOrderLocalID` varchar(16) NOT NULL,
  `ClientID` varchar(13) DEFAULT NULL,
  `OrderOperatorID` varchar(11) NOT NULL,
  `OrderLocalID` varchar(16) NOT NULL,
  `UsedMargin` double(16,4) DEFAULT NULL,
  `UsedFee` double(16,4) DEFAULT NULL,
  `SequenceNo` int(11) DEFAULT NULL,
  PRIMARY KEY (`ExchangeID`,`OrderSysID`,`TradeID`,`TradeDate`),
  KEY `id` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=51 DEFAULT CHARSET=utf8;

-- Data exporting was unselected.

-- Dumping structure for table jinchan.t_sync_instrument
CREATE TABLE IF NOT EXISTS `t_sync_instrument` (
  `ExchangeID` varchar(9) NOT NULL,
  `InstrumentID` varchar(11) NOT NULL,
  `InstrumentName` varchar(31) DEFAULT NULL,
  `ProductID` varchar(11) DEFAULT NULL,
  `ProductClass` char(1) NOT NULL,
  `LongPosLimit` int(8) DEFAULT NULL,
  `ShortPosLimit` int(8) DEFAULT NULL,
  `MaxOrderVolume` int(8) DEFAULT NULL,
  `MinOrderVolume` int(8) DEFAULT NULL,
  `VolumeMultiple` int(8) DEFAULT NULL,
  `PriceTick` double(12,3) DEFAULT NULL,
  PRIMARY KEY (`ExchangeID`,`InstrumentID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Data exporting was unselected.

-- Dumping structure for table jinchan.t_sync_investoraccount
CREATE TABLE IF NOT EXISTS `t_sync_investoraccount` (
  `BrokerID` varchar(11) NOT NULL,
  `InvestorID` varchar(11) NOT NULL DEFAULT '',
  `TradingDay` varchar(9) NOT NULL DEFAULT '',
  `PreBalance` double(16,4) DEFAULT NULL,
  `PreAvailable` double(16,4) DEFAULT NULL,
  `Balance` double(16,4) DEFAULT NULL,
  `Available` double(16,4) DEFAULT NULL,
  `Deposit` double(16,4) DEFAULT NULL,
  `Withdraw` double(16,4) DEFAULT NULL,
  `Margin` double(16,4) DEFAULT NULL,
  `Fee` double(16,4) DEFAULT NULL,
  `OtherFee` double(16,4) DEFAULT NULL,
  `CloseProfit` double(16,4) DEFAULT NULL,
  `PositionProfit` double(16,4) DEFAULT NULL,
  `FrozenMargin` double(16,4) DEFAULT NULL,
  `FrozenFee` double(16,4) DEFAULT NULL,
  `LongMargin` double(16,4) DEFAULT NULL,
  `ShortMargin` double(16,4) DEFAULT NULL,
  `LongFrozenMargin` double(16,4) DEFAULT NULL,
  `ShortFrozenMargin` double(16,4) DEFAULT NULL,
  `DynamicRights` double(16,4) DEFAULT NULL,
  `Risk` double(16,4) DEFAULT NULL,
  PRIMARY KEY (`BrokerID`,`InvestorID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Data exporting was unselected.

-- Dumping structure for table jinchan.t_sync_investorfee
CREATE TABLE IF NOT EXISTS `t_sync_investorfee` (
  `BrokerID` varchar(11) NOT NULL,
  `ExchangeID` varchar(9) NOT NULL,
  `InvestorID` varchar(11) NOT NULL,
  `InstrumentID` char(11) NOT NULL,
  `OpenFeeRate` double(12,6) DEFAULT NULL,
  `OpenFeeAmt` double(12,6) DEFAULT NULL,
  `OffsetFeeRate` double(12,6) DEFAULT NULL,
  `OffsetFeeAmt` double(12,6) DEFAULT NULL,
  `OffsetTodayFeeRate` double(12,6) DEFAULT NULL,
  `OffsetTodayFeeAmt` double(12,6) DEFAULT NULL,
  PRIMARY KEY (`BrokerID`,`ExchangeID`,`InvestorID`,`InstrumentID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Data exporting was unselected.

-- Dumping structure for table jinchan.t_sync_investorinfo
CREATE TABLE IF NOT EXISTS `t_sync_investorinfo` (
  `BrokerID` varchar(11) NOT NULL,
  `InvestorID` varchar(11) NOT NULL,
  `InvestorName` varchar(31) NOT NULL,
  `Password` varchar(16) NOT NULL,
  `TradeRight` char(1) NOT NULL DEFAULT '2',
  `IsActive` char(1) NOT NULL DEFAULT '1',
  `ClientID` varchar(13) NOT NULL,
  `IsNewAuth` char(1) NOT NULL DEFAULT '0',
  `Salt` varchar(32) DEFAULT NULL,
  `PasswordHash` varchar(128) DEFAULT NULL,
  PRIMARY KEY (`BrokerID`,`InvestorID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Data exporting was unselected.

-- Dumping structure for table jinchan.t_sync_investormargin
CREATE TABLE IF NOT EXISTS `t_sync_investormargin` (
  `BrokerID` varchar(11) NOT NULL,
  `ExchangeID` varchar(9) NOT NULL,
  `InvestorID` varchar(11) NOT NULL,
  `InstrumentID` char(11) NOT NULL,
  `LongMarginRate` double(12,6) DEFAULT NULL,
  `LongMarginAmt` double(12,6) DEFAULT NULL,
  `ShortMarginRate` double(12,6) DEFAULT NULL,
  `ShortMarginAmt` double(12,6) DEFAULT NULL,
  PRIMARY KEY (`BrokerID`,`ExchangeID`,`InvestorID`,`InstrumentID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Data exporting was unselected.

-- Dumping structure for table jinchan.t_sync_investorposition
CREATE TABLE IF NOT EXISTS `t_sync_investorposition` (
  `BrokerID` varchar(11) NOT NULL,
  `InvestorID` varchar(11) NOT NULL,
  `TradingDay` varchar(9) NOT NULL,
  `ExchangeID` varchar(9) DEFAULT NULL,
  `InstrumentID` varchar(11) NOT NULL,
  `ProductClass` char(1) NOT NULL,
  `Direction` char(1) NOT NULL,
  `UsedMargin` double(16,4) DEFAULT NULL,
  `Position` int(8) DEFAULT NULL,
  `PositionCost` double(16,4) DEFAULT NULL,
  `YdPosition` int(8) DEFAULT NULL,
  `YdPositionCost` double(16,4) DEFAULT NULL,
  `YdPositionPrice` double(16,4) DEFAULT NULL,
  `FrozenMargin` double(16,4) DEFAULT NULL,
  `FrozenFee` double(16,4) DEFAULT NULL,
  `FrozenPosition` int(8) DEFAULT NULL,
  `LastTradeID` varchar(16) DEFAULT NULL,
  `LastOrderLocalID` varchar(16) DEFAULT NULL,
  PRIMARY KEY (`BrokerID`,`InvestorID`,`TradingDay`,`InstrumentID`,`Direction`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Data exporting was unselected.

-- Dumping structure for table jinchan.t_sync_operatorinfo
CREATE TABLE IF NOT EXISTS `t_sync_operatorinfo` (
  `BrokerID` varchar(11) NOT NULL DEFAULT '',
  `OperatorID` varchar(11) NOT NULL DEFAULT '',
  `UserName` varchar(31) NOT NULL,
  `Password` varchar(16) NOT NULL,
  `PermissionRight` char(1) NOT NULL,
  PRIMARY KEY (`BrokerID`,`OperatorID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Data exporting was unselected.

-- Dumping structure for table jinchan.t_sync_operatorinvestor
CREATE TABLE IF NOT EXISTS `t_sync_operatorinvestor` (
  `BrokerID` varchar(11) NOT NULL,
  `OperatorID` varchar(11) NOT NULL,
  `InvestorID` varchar(11) NOT NULL,
  PRIMARY KEY (`BrokerID`,`OperatorID`,`InvestorID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Data exporting was unselected.

-- Dumping structure for table jinchan.t_sync_seatinfo
CREATE TABLE IF NOT EXISTS `t_sync_seatinfo` (
  `AppName` varchar(50) NOT NULL,
  `ExchangeID` varchar(9) NOT NULL DEFAULT '',
  `ParticipantID` varchar(11) NOT NULL DEFAULT '',
  `SeatID` varchar(11) NOT NULL DEFAULT '',
  `Password` varchar(16) DEFAULT NULL,
  `SeatStatus` char(1) NOT NULL,
  `TraderID` varchar(50) NOT NULL DEFAULT '',
  `ResumeType` char(1) DEFAULT '1',
  `FrontAddr` varchar(33) DEFAULT NULL,
  `OrderLocalID` varchar(16) DEFAULT NULL,
  PRIMARY KEY (`ExchangeID`,`ParticipantID`,`SeatID`,`AppName`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Data exporting was unselected.

-- Dumping structure for table jinchan.t_sync_systemstatus
CREATE TABLE IF NOT EXISTS `t_sync_systemstatus` (
  `TradingDay` varchar(9) NOT NULL DEFAULT '',
  `SystemStatus` char(1) NOT NULL,
  `TradingStatus` char(1) NOT NULL,
  PRIMARY KEY (`TradingDay`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- Data exporting was unselected.

/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IF(@OLD_FOREIGN_KEY_CHECKS IS NULL, 1, @OLD_FOREIGN_KEY_CHECKS) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
