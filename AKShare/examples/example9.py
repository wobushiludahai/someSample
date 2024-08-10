import akshare as ak
import pandas as pd

# 获取资金流向

# 设置 pandas 选项以显示所有行和列
pd.set_option('display.max_rows', None)  # 显示所有行
pd.set_option('display.max_columns', None)  # 显示所有列

# 同花顺
# symbol="即时"; choice of {“即时”, "3日排行", "5日排行", "10日排行", "20日排行"}
# 个股资金流
stock_fund_flow_individual_df = ak.stock_fund_flow_individual(symbol="即时")
print(stock_fund_flow_individual_df)

# symbol="即时"; choice of {“即时”, "3日排行", "5日排行", "10日排行", "20日排行"}
# 概念资金流
stock_fund_flow_concept_df = ak.stock_fund_flow_concept(symbol="即时")
print(stock_fund_flow_concept_df)

# symbol="即时"; choice of {“即时”, "3日排行", "5日排行", "10日排行", "20日排行"}
# 行业资金流
stock_fund_flow_industry = ak.stock_fund_flow_industry(symbol="即时")
print(stock_fund_flow_industry)

# 大单追踪
stock_fund_flow_big_deal_df = ak.stock_fund_flow_big_deal()
print(stock_fund_flow_big_deal_df)



# 东方财富
# 个股资金流
# stock="000425"; 股票代码
# market="sh"; 上海证券交易所: sh, 深证证券交易所: sz, 北京证券交易所: bj
stock_individual_fund_flow_df = ak.stock_individual_fund_flow(stock="600094", market="sh")
print(stock_individual_fund_flow_df)

# 个股资金流排名
# indicator="今日"; choice {"今日", "3日", "5日", "10日"}
# stock_individual_fund_flow_rank_df = ak.stock_individual_fund_flow_rank(indicator="今日")
# stock_individual_fund_flow_rank_df = ak.stock_individual_fund_flow_rank(indicator="3日")
# stock_individual_fund_flow_rank_df = ak.stock_individual_fund_flow_rank(indicator="5日")
stock_individual_fund_flow_rank_df = ak.stock_individual_fund_flow_rank(indicator="10日")
print(stock_individual_fund_flow_rank_df)

# 大盘资金流
stock_market_fund_flow_df = ak.stock_market_fund_flow()
print(stock_market_fund_flow_df)

# 板块资金流
stock_sector_fund_flow_rank_df = ak.stock_sector_fund_flow_rank(indicator="今日", sector_type="行业资金流")
print(stock_sector_fund_flow_rank_df)

# 主力净流入排名
# symbol="全部股票"；choice of {"全部股票", "沪深A股", "沪市A股", "科创板", "深市A股", "创业板", "沪市B股", "深市B股"}
stock_main_fund_flow_df = ak.stock_main_fund_flow(symbol="全部股票")
print(stock_main_fund_flow_df)

# 行业个股资金流
# symbol="电源设备"
# indicator="今日"; choice of {"今日", "5日", "10日"}
stock_sector_fund_flow_summary_df = ak.stock_sector_fund_flow_summary(symbol="电源设备", indicator="今日")
print(stock_sector_fund_flow_summary_df)





# 筹码分布，获取获利占比
# adjust=""; choice of {"qfq": "前复权", "hfq": "后复权", "": "不复权"}
stock_cyq_em_df = ak.stock_cyq_em(symbol="000001", adjust="")
print(stock_cyq_em_df)