import akshare as ak
import pandas as pd

# 龙虎榜

# 设置 pandas 选项以显示所有行和列
# pd.set_option('display.max_rows', None)  # 显示所有行
# pd.set_option('display.max_columns', None)  # 显示所有列

stock_lhb_detail_em_df = ak.stock_lhb_detail_em(start_date="20230403", end_date="20230417")
print(stock_lhb_detail_em_df)

#个股上榜统计
# symbol="近一月"; choice of {"近一月", "近三月", "近六月", "近一年"}
stock_lhb_stock_statistic_em_df = ak.stock_lhb_stock_statistic_em(symbol="近一月")
print(stock_lhb_stock_statistic_em_df)

# 机构买卖每日统计
stock_lhb_jgmmtj_em_df = ak.stock_lhb_jgmmtj_em(start_date="20240417", end_date="20240430")
print(stock_lhb_jgmmtj_em_df)

# 机构席位追踪
# symbol="近一月"; choice of {"近一月", "近三月", "近六月", "近一年"}
stock_lhb_jgstatistic_em_df = ak.stock_lhb_jgstatistic_em(symbol="近一月")
print(stock_lhb_jgstatistic_em_df)

# 每日活跃营业部
stock_lhb_hyyyb_em_df = ak.stock_lhb_hyyyb_em(start_date="20220324", end_date="20220324")
print(stock_lhb_hyyyb_em_df)

# 营业部排行
# symbol="近一月"; choice of {"近一月", "近三月", "近六月", "近一年"}
stock_lhb_yybph_em_df = ak.stock_lhb_yybph_em(symbol="近一月")
print(stock_lhb_yybph_em_df)

# 营业部统计
# symbol="近一月"; choice of {"近一月", "近三月", "近六月", "近一年"}
stock_lhb_traderstatistic_em_df = ak.stock_lhb_traderstatistic_em(symbol="近一月")
print(stock_lhb_traderstatistic_em_df)

# 个股龙虎榜详情
stock_lhb_stock_detail_em_df = ak.stock_lhb_stock_detail_em(symbol="600077", date="20070416", flag="买入")
print(stock_lhb_stock_detail_em_df)

# 还有针对营业部的金额及实力排行