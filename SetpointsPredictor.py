import numpy as np
import math
import datetime, pytz
import yaml
from datetime import timedelta
from xbos import get_client
from xbos.services import mdal
import pandas as pd
from scipy.spatial import distance

def euclidian_distance(a, b):
	return distance.euclidean(a, b)

class SetpointsPredictor:

	def __init__(self, yaml_file):

		with open(yaml_file, 'r') as ymlfile:
			cfg = yaml.load(ymlfile)

		self.pytz_timezone = cfg["Data_Manager"]["Pytz_Timezone"]
		self.zone = cfg["Data_Manager"]["Zone"]
		self.interval = cfg["Interval_Length"]

		if cfg["Data_Manager"]["Server"]:
			self.c = get_client(agent = cfg["Data_Manager"]["Agent_IP"], entity=cfg["Data_Manager"]["Entity_File"])
		else:
			self.c = get_client()

		self.df = self.get_setpoints(datetime.datetime.utcnow().replace(tzinfo=pytz.timezone("UTC")))

	def get_setpoints(self, now):
		thermostat_high = 'dbbf4a91-107a-3b15-b2c0-a49b54116daa'
		thermostat_low = 'eeadc8ed-6255-320d-b845-84f44748fe95'
		uuids = [thermostat_high, thermostat_low]

		c = mdal.MDALClient("xbos/mdal", client=self.c)
		dfs = c.do_query({'Composition': uuids,
						  'Selectors': [mdal.MEAN]*len(uuids),
						  'Time': {'T0': (now-timedelta(days=25)).strftime('%Y-%m-%d %H:%M:%S') + ' UTC',
								   'T1': now.strftime('%Y-%m-%d %H:%M:%S') + ' UTC',
								   'WindowSize': str(self.interval)+'min',
								   'Aligned': True}})

		df = pd.concat([dframe for uid, dframe in dfs.items()], axis=1)
		df = df.rename(columns={uuids[0]: 't_high', uuids[1]: 't_low'})
		df = df.fillna(method='pad')

		return df.tz_localize(None)


	def mins_in_day(self, timestamp):
		return timestamp.hour * 60 + timestamp.minute
	


	def find_similar_days(self, training_data, now, observation_length, k, method = euclidian_distance):

		min_time = training_data.index[0] + timedelta(minutes=observation_length)
		selector = ((training_data.index.minute == now.minute) &
					(training_data.index.hour == now.hour) &
					(training_data.index >= min_time))

		similar_moments = training_data[selector][:-1]
		obs_td = timedelta(minutes=observation_length)

		similar_moments['Similarity'] = [
			method(training_data[(training_data.index >= now - obs_td) &
								 (training_data.index <= now)].get_values(),
				   training_data[(training_data.index >= i - obs_td) &
								 (training_data.index <= i)].get_values()
			) for i in similar_moments.index
			]

		indexes = (similar_moments.sort_values('Similarity', ascending=True).head(k).index)
		return indexes

	def predict(self, data, now, similar_moments, prediction_time, resample_time):

		prediction = np.zeros((int(math.ceil(prediction_time/resample_time)) + 1, len(data.columns)))
		for i in similar_moments:
			prediction += float(1.0 / float(len(similar_moments))) * data[(data.index >= i) & (data.index <= i + timedelta(minutes=prediction_time))]

		prediction[0] = data[data.index == now]
		time_index = pd.date_range(now, now+timedelta(minutes=prediction_time),freq='15T')
		return pd.DataFrame(data=prediction, index=time_index)

	def predictions(self):
		now = self.df.index[-1]
		observation_length_addition = 4*60
		k = 3
		prediction_time = 4*60
		resample_time = self.interval

		observation_length = self.mins_in_day(now) + observation_length_addition
		similar_moments_thigh = self.find_similar_days(self.df['t_high'].to_frame(), now, observation_length, k)
		predictions_thigh = self.predict(self.df['t_high'].to_frame(), now, similar_moments_thigh, prediction_time, resample_time)
		similar_moments_tlow = self.find_similar_days(self.df['t_low'].to_frame(), now, observation_length, k)
		predictions_tlow = self.predict(self.df['t_low'].to_frame(), now, similar_moments_tlow, prediction_time, resample_time)

		return predictions_thigh, predictions_tlow


if __name__ == '__main__':

	sp = SetpointsPredictor("config_south.yml")
	print sp.predictions()