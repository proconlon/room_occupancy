# This script generated the line plot for the 30 trials we ran for sensor/code reliability.
# In the Result column, the pattern should be Increment, Decrement, Increment, Decrement,... and so on for all trials. 

import matplotlib.pyplot as plt
import pandas as pd

data = {
    'Trial': range(1, 31), # Trial numbers (10 for each pace)
    'Direction': ['Entry', 'Exit']*15, # Test cases were alternated between Entry and Exit (so cumulative count is 0)
    'Result': ['Increment', 'Decrement', 'Increment', 'Decrement', 'Increment', 'Nothing','Increment', 'Decrement', 'Increment', 'Decrement',  # Walking
               'Increment', 'Decrement', 'Increment', 'Decrement', 'Nothing', 'Decrement', 'Increment', 'Decrement','Increment', 'Increment',  # Late to Class
               'Increment', 'Increment', 'Nothing', 'Decrement','Increment', 'Decrement', 'Increment', 'Decrement', 'Increment', 'Decrement'], # Boston Marathon
    'Pace': ['Walking']*10 + ['Late to Class']*10 + ['Boston Marathon']*10
}

df = pd.DataFrame(data)

# Basic map of real life result (inc, dec, nothing) to numerical values (1, -1, 0)
df['Result_Value'] = df['Result'].map({'Increment': 1, 'Decrement': -1, 'Nothing': 0, 'Entry': 1, 'Exit': -1})

# gouped by pace, starting at 0
df['Cumulative'] = df.groupby('Pace')['Result_Value'].cumsum() - 1

# plot
fig, ax = plt.subplots(figsize=(10, 6))

colors = {'Walking': 'royalblue', 'Late to Class': 'darkorange', 'Boston Marathon': 'forestgreen'}
for pace, group_data in df.groupby('Pace'):
    ax.plot(group_data['Trial'], group_data['Cumulative'], marker='o', linestyle='-', color=colors[pace], label=pace)

ax.set_title('Sensor Reliability Across Different Speeds')
ax.set_xlabel('Trial')
ax.set_ylabel('Cumulative Change in Count')
ax.legend(title='Pace', loc='lower right')

plt.xticks(df['Trial'])
plt.yticks(range(int(df['Cumulative'].min()), int(df['Cumulative'].max()) + 1))  # y grid is integer no .5 or anything
plt.grid(visible=True)
plt.tight_layout()
plt.show()