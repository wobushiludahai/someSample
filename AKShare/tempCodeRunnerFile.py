df = df.set_index('季度')
# df.index = pd.to_datetime(df.index)
# gdp = df['国内生产总值-绝对值'][::-1].astype('float')

# # GDP DIFF
# gdp_diff = gdp.diff(4).dropna()