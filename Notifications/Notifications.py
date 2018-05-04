#################################################################################
#																				#
#						ThermoCat Notification Generator						#
#																				#
#################################################################################
import pandas as pd

# Reading statistical data (http://www.cpuc.ca.gov/res2016/) [TODO: include all data in the csv file]
UsageStatistics = pd.read_csv('2016 Residential Electric Usage and Bill Data Statistics by Climate Zone.csv', index_col=0, header=[0,1,2])

# Reading Zip2zone data (http://www.cpuc.ca.gov/res2016/)
Zip2zone = pd.read_csv('Zip2zone.csv', index_col=0, header=[0])


# Convert month to season
def Month2Season(Month):
	"""
	inputs:
		Month: Integer [1-12]
	"""
	if Month in [1,2,3,10,11,12]:
		return "Winter"
	else:	
		return "Summer"


# Convert Zip code to Zone
def Zip2Zone(Zip):
	"""
	inputs:
		Zip: Integer
	"""
	return Zip2zone['Zone'][Zip]


# Produce notifications
def SavingNotification(Zip, Month, Bill, Rate):
	"""
	inputs:
		Zip: Integer
		Month: Integer [1-12]
		Bill: Integer in $
		Rate: String {"D","D-CARE"} 
	"""

	AverageBill = float(UsageStatistics['D'][str(Zip2Zone(Zip))][str(Month2Season(Month))]['Bill(Avg)'][1:])

	return Bill<AverageBill


############################## example
Zip = 90002
Month = 1
Bill = 410
Rate = "D"
print SavingNotification(Zip, Month, Bill, Rate)
