import akshare as ak
import pandas as pd

# 获取财务数据

# 设置 pandas 选项以显示所有行和列
pd.set_option('display.max_rows', None)  # 显示所有行
pd.set_option('display.max_columns', None)  # 显示所有列

# 业绩报表
stock_yjbb_em_df = ak.stock_yjbb_em(date="20240331")
print(stock_yjbb_em_df)

# 业绩快报
stock_yjkb_em_df = ak.stock_yjkb_em(date="20240630")
print(stock_yjkb_em_df)

# 资产负债表
stock_zcfz_em_df = ak.stock_zcfz_em(date="20220331")
print(stock_zcfz_em_df)

# 利润表
stock_lrb_em_df = ak.stock_lrb_em(date="20220331")
print(stock_lrb_em_df)

# 现金流量表
stock_xjll_em_df = ak.stock_xjll_em(date="20220331")
print(stock_xjll_em_df)