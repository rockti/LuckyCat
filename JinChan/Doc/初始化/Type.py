import xlrd
from typing import NamedTuple
from enum import Enum
from recordclass import RecordClass,asdict

class Direction(Enum):
    Buy = '0'
    Sell = '1'
class ProductClass(Enum):
    Defer = '1'
    Spot = '2'

class ExcelReader:
    def __init__(self, *args, **kw):
        self.header = args[0]
        self.index = {k: v for v, k in enumerate(self.header)}
    def validate_header(self, header):
        for i in range(len(header)):
            if (header[i].value != self.header[i]):
                return False
        return True
        
    def row_value(self,row,name):
        return row[self.index[name]].value
class Investor(RecordClass):
    BrokerID:str = ""
    InvestorID:str = ""
    InvestorName:str = ""
    Password:str = ""
    ClientID:str = ""
    IsActive:str = "0"
    IsNewAuth:str = "0"
    Salt:str = ""
    PasswordHash = ""

class InvestorReader(ExcelReader):
    
    def __init__(self, *args, **kw):
        investor_header = ["序号","席位代码","客户号","简称","密码","交易编码","状态"]
        super(InvestorReader, self).__init__(investor_header)

    def load(self,filename):
        investors = []
        wb = xlrd.open_workbook(filename) 
        sheet = wb.sheet_by_index(0) 
        for i in range(1,sheet.nrows): 
            row = sheet.row(i)
            investor = Investor(
                BrokerID = self.row_value(row,"席位代码"),
                InvestorID = self.row_value(row,"客户号"),
                InvestorName = self.row_value(row,"简称"),
                Password = self.row_value(row,"密码"),
                ClientID = self.row_value(row,"交易编码"),
                IsActive = "0" if self.row_value(row,"状态") == "可用" else "1"
            )
            investors.append(investor)
        return investors


class Account(RecordClass):
    BrokerID:str = ""
    InvestorID:str = ""
    TradingDay:str = ""
    PreBalance:float = 0
    PreAvailable:float = 0
    Balance:float = 0.0
    Available:float = 0.0
    Deposit:float = 0.0
    Withdraw:float = 0.0
    Margin:float = 0.0
    Fee:float = 0.0
    OtherFee:float = 0.0
    CloseProfit:float = 0.0
    PositionProfit:float = 0.0
    FrozenMargin:float = 0.0
    FrozenFee:float = 0.0
    LongMargin:float = 0.0
    ShortMargin:float = 0.0
    LongFrozenMargin:float = 0.0
    ShortFrozenMargin:float = 0.0
    DynamicRights:float = 0.0
    Risk:float = 0.0

class AccountReader(ExcelReader):
    def __init__(self, *args, **kw):
        account_header = ["序号","客户号","简称","客户类型","交易日期","上日结存","当日入金","当日出金","持仓保证金","交割货款","今日盈亏",
        "浮动盈亏","客户手续费","席位手续费","手续费(会员收取)","递延费","席位交割费","客户交割费","仓储费","运保费","溢短金额","出入库费",
        "基础保证金","今日结存","客户权益","可用资金","可用资金及额度","追加保证金","客户租借费","席位租借费","租借费（会员收取）"]
        super(AccountReader, self).__init__(account_header)

    def load(self,filename):
        accounts = []
        wb = xlrd.open_workbook(filename) 
        sheet = wb.sheet_by_index(0) 
        for i in range(1,sheet.nrows): 
            row = sheet.row(i)
            account = Account(
                InvestorID = self.row_value(row,"客户号"),
                PreBalance = float(self.row_value(row,"今日结存")),
                Margin = float(self.row_value(row,"持仓保证金"))
            )
            accounts.append(account)
        return accounts


class Position(RecordClass):
    BrokerID:str = ""
    InvestorID:str = ""
    TradingDay:str = ""
    ExchangeID:str = "SGE"
    InstrumentID:str = ""
    ProductClass:str = ""
    Direction:str = ""
    UsedMargin:float = 0.0
    Position:int = 0
    PositionCost:float = 0.0
    YdPosition:int = 0
    YdPositionPrice:float = 0.0
    FrozenMargin:float = 0.0
    FrozenFee:float = 0.0
    FrozenPosition:int = 0

class PositionReader:
    spot_id_mapping = {
        201:"Au99.99",
        202:"Au99.95",
        203:"Pt99.95",
        204:"Au50g",
        207:"Au100g",
        210:"Au99.5", 
        301:"iAu99.99",
        302:"iAu99.5",
        303:"iAu100g", 
        }

    def spot_id_to_inst(self,id):
        if int(id) in self.spot_id_mapping:
            return self.spot_id_mapping[int(id)]
        return ""
    spot_position_header= ["序号","交易日期","客户号","客户交易编码","客户简称 ","交易所代码","席位号","交割品种代码","库存总量","可用库存",
    "充抵库存","待提库存","质押库存","法律冻结库存","当日买入","当日卖出","当日存入","当日存入实重","入库溢短总量","未清算入库溢短",
    "当日提出","当日提出实重","出库溢短总量","当日借出","当日借入","当日转出","当日转入","现货溢短重量","现货溢短金额",
    "递延溢短重量","递延溢短金额","过户业务冻结库存"]
    spot_reader = ExcelReader(spot_position_header)

    defer_position_header=["序号","日期","客户号","简称","客户类","交易所","品种","到期日","买持量","卖持量","买保证金","卖保证金","总保证金","清算价","交易编码"]
    defer_reader = ExcelReader(defer_position_header)
    def kg_to_position(self,insts,instCode,kg):
        for inst in insts:
            if inst.InstrumentID == instCode:
                return kg*1000 / inst.VolumeMultiple
        return 0
    def spot_value(self,row,name):
        return self.spot_reader.row_value(row,name)
    def defer_value(self,row,name):
        return self.defer_reader.row_value(row,name)

    def validate_spot_header(self,header):
        return self.spot_reader.validate_header(header)
    def validate_defer_header(self,header):
        return self.defer_reader.validate_header(header)
    def defer_from(self,filename):
        positions = []
        wb = xlrd.open_workbook(filename) 
        sheet = wb.sheet_by_index(0) 
        for i in range(1,sheet.nrows): 
            row = sheet.row(i)
            long_pos:int = self.defer_value(row,"买持量")
            short_pos:int = self.defer_value(row,"卖持量")
            if long_pos > 0:
                long = Position(
                    ProductClass = ProductClass.Defer,
                    InvestorID = self.defer_value(row,"客户号"),
                    InstrumentID = self.defer_value(row,"品种"),
                    Direction = Direction.Buy,
                    YdPosition = self.defer_value(row,"买持量"),
                    YdPositionPrice = self.defer_value(row,"清算价"),
                    UsedMargin = self.defer_value(row,"买保证金")
                )
                positions.append(long)
            if short_pos > 0:
                short = Position(
                    ProductClass = ProductClass.Defer,
                    InvestorID = self.defer_value(row,"客户号"),
                    InstrumentID = self.defer_value(row,"品种"),
                    Direction = Direction.Sell,
                    YdPosition = self.defer_value(row,"卖持量"),
                    YdPositionPrice = self.defer_value(row,"清算价"),
                    UsedMargin = self.defer_value(row,"卖保证金")
                )
                positions.append(short)
        return positions
    def spot_from(self, filename, insts):
        positions = []
        wb = xlrd.open_workbook(filename) 
        sheet = wb.sheet_by_index(0) 
        for i in range(1,sheet.nrows): 
            row = sheet.row(i)
            kg_pos = self.spot_value(row,"可用库存")
            instCode = self.spot_id_to_inst(self.spot_value(row,"交割品种代码"))
            instPos = self.kg_to_position(insts,instCode,kg_pos)
            if instPos > 0 and instCode != "":
                pos = Position(
                    Direction = Direction.Buy,
                    ProductClass = ProductClass.Spot,
                    InvestorID = self.defer_value(row,"客户号"),
                    InstrumentID = instCode,
                    YdPosition = instPos
                )
                positions.append(pos)   
        return positions     


class Instrument(RecordClass):
    ExchangeID:str = "SGE"
    InstrumentID:str = ""
    InstrumentName:str = ""
    ProductID:str = ""
    ProductClass:str = ""
    LongPosLimit:int = 0
    ShortPosLimit:int = 0
    MaxOrderVolume:int = 0
    MinOrderVolume:int = 0
    VolumeMultiple:int = 0
    PriceTick:float = 0.0

class InstrumentReader(ExcelReader):
    def __init__(self, *args, **kw):
        header = ["序号","品种代码","品种名称","交易所代码","市场代码","品种类别","交割品种代码","合约交易状态","品种活动状态","交易单位","最小变动价位",
                "报价单位","最小申报数量","最大申报数量","最大持仓量","涨停板率","跌停板率"]
        super(InstrumentReader, self).__init__(header)                    
    def product_class(self,name):
        if name == "递延":
            return ProductClass.Defer
        if name == "现货":
            return ProductClass.Spot
    def load(self,filename, productClass):
        insts = []
        wb = xlrd.open_workbook(filename) 
        sheet = wb.sheet_by_index(0) 
        for i in range(1,sheet.nrows): 
            row = sheet.row(i)
            inst = Instrument(
                InstrumentID=self.row_value(row,"品种代码"),
                InstrumentName=self.row_value(row,"品种名称"),
                ProductClass = self.product_class(self.row_value(row,"市场代码")),
                ProductID = self.row_value(row,"交割品种代码"),
                MaxOrderVolume=self.row_value(row,"最大申报数量"),
                MinOrderVolume=self.row_value(row,"最小申报数量"),
                PriceTick=self.row_value(row,"最小变动价位"),
                VolumeMultiple=self.row_value(row,"交易单位"),
                LongPosLimit=self.row_value(row,"最大持仓量"),
                ShortPosLimit=1
            )
            insts.append(inst)
        return insts

class Fee(RecordClass):
    BrokerID:str = ""
    ExchangeID:str = "SGE"
    InvestorID:str = ""
    InstrumentID:str = ""
    OpenFeeRate:float = 0.0
    OpenFeeAmt:float = 0.0
    OffsetFeeRate:float = 0.0
    OffsetFeeAmt:float = 0.0
    OffsetTodayFeeRate:float = 0.0
    OffsetTodayFeeAmt:float = 0.0

class FeeReader(ExcelReader):
    def __init__(self, *args, **kw):
        header = ["序号","客户号/类","客户号/类简称","交易所代码","品种代码","品种简称","席位代码","席位简称","开/平/强平","买手续费","卖手续费","设置人员","设置日期"]
        super(FeeReader, self).__init__(header)                    
    def load(self,filename):
        fees = []
        feeMap = {}
        wb = xlrd.open_workbook(filename) 
        sheet = wb.sheet_by_index(0) 
        for i in range(1,sheet.nrows): 
            row = sheet.row(i)
            fee = Fee(
                BrokerID = self.row_value(row,"席位代码"),
                InvestorID = self.row_value(row,"客户号/类"),
                InstrumentID = self.row_value(row,"品种代码"),
                OpenFeeRate = self.row_value(row,"买手续费"),
                OffsetFeeRate = self.row_value(row, "卖手续费"),
                OffsetTodayFeeRate = self.row_value(row, "卖手续费")
            )
            flag = self.row_value(row,"开/平/强平")
            key = fee.BrokerID + "-" + fee.InvestorID + "-"+ fee.InstrumentID
            if key in feeMap and flag == "平":
                existedFee = feeMap[key]
                existedFee.OffsetFeeRate = fee.OffsetFeeRate
                existedFee.OffsetTodayFeeRate = fee.OffsetFeeRate
                fees.append(existedFee)
            if flag == "":
                fees.append(fee)
            if flag == "开":
                feeMap[key] = fee
        return fees

class Margin(RecordClass):
    BrokerID:str = ""
    ExchangeID:str = "SGE"
    InvestorID:str = ""
    InstrumentID:str = ""
    LongMarginRate:float = 0.0
    LongMarginAmt:float = 0.0
    ShortMarginRate:float = 0.0
    ShortMarginAmt:float = 0.0

class MarginReader(ExcelReader):
    def __init__(self, *args, **kw):
        header = ["序号","客户号/类","客户号/类简称","交易所代码","品种代码","品种简称","席位代码","席位简称","买保证金","卖保证金","设置人员","设置日期"]
        super(MarginReader, self).__init__(header)                    
    def load(self,filename):
        margins = []
        wb = xlrd.open_workbook(filename) 
        sheet = wb.sheet_by_index(0) 
        for i in range(1,sheet.nrows): 
            row = sheet.row(i)
            margin = Margin(
                BrokerID = self.row_value(row,"席位代码"),
                InvestorID = self.row_value(row,"客户号/类"),
                InstrumentID = self.row_value(row,"品种代码"),
                LongMarginRate = self.row_value(row,"买保证金"),
                ShortMarginRate = self.row_value(row, "卖保证金")
            )
            margins.append(margin)
        return margins

class ExcelProcessor:    

    def validate_investor_file(self, filename):
        self.investorReader = InvestorReader()
        wb = xlrd.open_workbook(filename) 
        sheet = wb.sheet_by_index(0) 
        result = self.investorReader.validate_header(sheet.row(0))
        if result:
            self.investor_file = filename
        return result

    def validate_account_file(self, filename):
        self.accountReader = AccountReader()
        wb = xlrd.open_workbook(filename) 
        sheet = wb.sheet_by_index(0) 
        result = self.accountReader.validate_header(sheet.row(0))
        if result:
            self.account_file = filename
        return result
    def validate_defer_position_file(self, filename):
        self.positionReader = PositionReader()
        wb = xlrd.open_workbook(filename) 
        sheet = wb.sheet_by_index(0) 
        result = self.positionReader.validate_defer_header(sheet.row(0))
        if result:
            self.defer_postion_file = filename
        return result
    def validate_spot_position_file(self, filename):
        self.positionReader = PositionReader()
        wb = xlrd.open_workbook(filename) 
        sheet = wb.sheet_by_index(0) 
        result = self.positionReader.validate_spot_header(sheet.row(0))
        if result:
            self.spot_position_file = filename
        return result

    def validate_defer_inst_file(self,filename):
        self.instReader = InstrumentReader()
        wb = xlrd.open_workbook(filename) 
        sheet = wb.sheet_by_index(0) 
        result = self.instReader.validate_header(sheet.row(0))
        if result:
            self.defer_inst_file = filename
        return result
    def validate_spot_inst_file(self,filename):
        self.instReader = InstrumentReader()
        wb = xlrd.open_workbook(filename) 
        sheet = wb.sheet_by_index(0) 
        result = self.instReader.validate_header(sheet.row(0))
        if result:
            self.spot_inst_file = filename
        return result
    def validate_margin_file(self,filename):
        self.marginReader = MarginReader()
        wb = xlrd.open_workbook(filename) 
        sheet = wb.sheet_by_index(0) 
        result = self.marginReader.validate_header(sheet.row(0))
        if result:
            self.margin_file = filename
        return result
    def validate_fee_file(self,filename):
        self.feeReader = FeeReader()
        wb = xlrd.open_workbook(filename) 
        sheet = wb.sheet_by_index(0) 
        result = self.feeReader.validate_header(sheet.row(0)) 
        if result:
            self.fee_file = filename
        return result
    def all_file_selected(self):
        if not hasattr(self,'fee_file'):
            return False
        if not hasattr(self,'margin_file'):
            return False
        if not hasattr(self,'defer_inst_file'):
            return False
        if not hasattr(self,'spot_inst_file'):
            return False
        if not hasattr(self,'spot_position_file'):
            return False
        if not hasattr(self,'defer_postion_file'):
            return False
        if not hasattr(self,'account_file'):
            return False
        if not hasattr(self,'investor_file'):
            return False
        return True
    def load_investors(self):
        return self.investorReader.load(self.investor_file)
    def load_insts(self):     
        defer_inst = self.instReader.load(self.defer_inst_file,ProductClass.Defer)
        spot_inst = self.instReader.load(self.spot_inst_file, ProductClass.Spot)
        return defer_inst + spot_inst
    def load_accounts(self,investorBrokers):
        accounts =  self.accountReader.load(self.account_file)
        for acc in accounts:
            acc.BrokerID = investorBrokers[acc.InvestorID]
        return accounts
    def load_fees(self, investorBrokers):
        fees = self.feeReader.load(self.fee_file)
#        for fee in fees:
#            if fee.InvestorID == "*":
#                fee.BrokerID = "*"
#            else: 
#                fee.BrokerID = investorBrokers[fee.InvestorID]
        return fees
    def load_margin(self, investorBrokers):
        margins = self.marginReader.load(self.margin_file)
#        for margin in margins:
#            if margin.InvestorID == "*":
#               margin.BrokerID = "*"
#            else: 
#                margin.BrokerID = investorBrokers[margin.InvestorID]
        return margins
    def load_positions(self,insts, investorBrokers):
        defer = self.positionReader.defer_from(self.defer_postion_file)
        spot = self.positionReader.spot_from(self.spot_position_file,insts)
        results = defer + spot
        for pos in results:
            pos.BrokerID = investorBrokers[pos.InvestorID]
        return results

