import numpy as np
import pandas as pd
from pylab import mpl, plt
import os

plt.style.use('ggplot')  # 设置matplotlib样式
mpl.rcParams['font.family'] = ['serif']  # 指定默认字体

file_path = os.path.abspath(os.path.dirname(__file__))

print(file_path)