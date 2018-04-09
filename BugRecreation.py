from xbos.services import mdal
import pandas as pd


def get_setpoints():
	thermostat_high = 'dbbf4a91-107a-3b15-b2c0-a49b54116daa'   # this is the Conference Room Cooling Setpoint data stream
	thermostat_low = 'eeadc8ed-6255-320d-b845-84f44748fe95'   # this is the Conference Room Heating Setpoint data stream
	uuids = [thermostat_high, thermostat_low]

	c = mdal.MDALClient("xbos/mdal")
	dfs = c.do_query({'Composition': uuids,
					  'Selectors': [mdal.MEAN] * len(uuids),
					  'Time': {'T0': '2018-03-11 09:00:00 UTC',
							   'T1': '2018-03-11 11:00:00 UTC',
							   'WindowSize': '15min',
							   'Aligned': True}})

	df = pd.concat([dframe for uid, dframe in dfs.items()], axis=1)
	df = df.rename(columns={uuids[0]: 't_high', uuids[1]: 't_low'})

	return df


if __name__ == '__main__':

	print get_setpoints()
	# the problem is one hour is being skipped. should have at least brought back 0 or NaN
