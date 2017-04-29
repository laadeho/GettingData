#include "pch.h"

#include "DataModel.h"

#include <string>

#include "muse.h"

using namespace interaxon::bridge;

DataModel::DataModel() {

    // EEG packets have the most data of all packet types
    // and EEG packets have a maximum of 6 pieces of data
    // (1 per channel).  Initialize our data buffer to have
    // adequate room to store the data from every packet type.
    data_buffer_	= new double[6];
	mDataAccel		= new double[6];
	mDataBattery	= new double[6];
	mDataEgg		= new double[6];
	mDataGyro		= new double[6];
	mDataDrlRef		= new double[6];
	mDataEggAlpha	= new double[6];
	mDataEggBeta	= new double[6];
	mDataEggDelta	= new double[6];
	mDataEggTheta	= new double[6];
	mDataEggGamma	= new double[6];
	
    clear();
}

DataModel::~DataModel() {
    delete[] data_buffer_;
}

bool DataModel::is_dirty() const {
    return dirty_;
}

void DataModel::clear_dirty_flag() {
    dirty_ = false;

}

void DataModel::setAllDirty(bool dirty)
{
	dirty_ = true;
	mDirtyAccel = true;
	mDirtyBattery = true;
	mDirtyEgg = true;
	mDirtyGyro = true;
	mDirtyDrlRef = true;
	mDirtyEggAlpha = true;
	mDirtyEggBeta = true;
	mDirtyEggDelta = true;
	mDirtyEggTheta = true;
	mDirtyEggGamma = true;

}

void DataModel::clear() {
    dirty_ = false;
	mDirtyAccel = false;
	mDirtyBattery = false;
	mDirtyEgg = false;
	mDirtyGyro = false;
	mDirtyDrlRef = false;

	mDirtyEggAlpha = false;
	mDirtyEggBeta  = false;
	mDirtyEggDelta = false;
	mDirtyEggTheta = false;
	mDirtyEggGamma = false;

    connection_state_string_ = "Unknown";
    version_string_ = "Unknown";
    for (int i = 0; i < 6; i++) {
        data_buffer_[i] = 0.0;
		mDataAccel[i] =	0.0;
		mDataBattery[i] =	0.0;
		mDataEgg[i] =		0.0;
		mDataGyro[i] =		0.0;
		mDataDrlRef[i] =	0.0;
		mDataEggAlpha[i] =	0.0;
		mDataEggBeta[i] =	0.0;
		mDataEggDelta[i] =	0.0;
		mDataEggTheta[i] =	0.0;
		mDataEggGamma[i] =	0.0;
    }
}

// add size_t for length of array
void DataModel::get_buffer(double *buffer) const
{
    for (int i = 0; i < 6; i++) {
        buffer[i] = data_buffer_[i];
    }
}

void DataModel::getBufferAccel(double * buffer) const
{
	for (int i = 0; i < 6; i++) {
		buffer[i] = mDataAccel[i];
	}
}

void DataModel::getBufferBattery(double * buffer) const
{
	for (int i = 0; i < 6; i++) {
		buffer[i] = mDataBattery[i];
	}
}

void DataModel::getBufferDrlRef(double * buffer) const
{
	for (int i = 0; i < 6; i++) {
		buffer[i] = mDataDrlRef[i];
	}
}

void DataModel::getBufferGyro(double * buffer) const
{
	for (int i = 0; i < 6; i++) {
		buffer[i] = mDataGyro[i];
	}
}

void DataModel::getBufferEggAplha(double * buffer) const
{
	for (int i = 0; i < 6; i++) {
		buffer[i] = mDataEggAlpha[i];
	}
}

void DataModel::getBufferEggBeta(double * buffer) const
{
	for (int i = 0; i < 6; i++) {
		buffer[i] = mDataEggBeta[i];
	}
}


void DataModel::getBufferEggDelta(double * buffer) const
{
	for (int i = 0; i < 6; i++) {
		buffer[i] = mDataEggDelta[i];
	}
}

void DataModel::getBufferEggTheta(double * buffer) const
{
	for (int i = 0; i < 6; i++) {
		buffer[i] = mDataEggTheta[i];
	}
}

void DataModel::getBufferEggGamma(double * buffer) const
{
	for (int i = 0; i < 6; i++) {
		buffer[i] = mDataEggGamma[i];
	}
}

const std::string &DataModel::get_connection_state() const {
    return connection_state_string_;
}
const std::string &DataModel::get_version() const {
    return version_string_;
}

void DataModel::set_values(const std::shared_ptr<MuseDataPacket> &p) {
    switch (p->packet_type()) {
    case MuseDataPacketType::ACCELEROMETER:
        set_accel_data(p);
        break;
    case MuseDataPacketType::BATTERY:
        set_battery_data(p);
        break;
    case MuseDataPacketType::DRL_REF:
        set_drl_ref_data(p);
        break;
    case MuseDataPacketType::GYRO:
        set_gyro_data(p);
        break;
	case MuseDataPacketType::ALPHA_RELATIVE:
		setEggData(p, mDataEggAlpha, &mDirtyEggAlpha);
		break;
	case MuseDataPacketType::BETA_RELATIVE:
		setEggData(p, mDataEggBeta, &mDirtyEggBeta);
		break;
	case MuseDataPacketType::DELTA_RELATIVE:
		setEggData(p, mDataEggDelta, &mDirtyEggDelta);
		break;
	case MuseDataPacketType::THETA_RELATIVE:
		setEggData(p, mDataEggTheta, &mDirtyEggTheta);
		break;
	case MuseDataPacketType::GAMMA_RELATIVE:
		setEggData(p, mDataEggGamma, &mDirtyEggGamma);
		break;
    default:
        set_eeg_data(p);
        break;
    }
    dirty_ = true;
}

void DataModel::set_values(const MuseArtifactPacket & p) {
    data_buffer_[0] = p.headband_on ? 1.0 : 0.0;
    data_buffer_[1] = p.blink ? 1.0 : 0.0;
    data_buffer_[2] = p.jaw_clench ? 1.0 : 0.0;
    data_buffer_[3] = 0.0;
    data_buffer_[4] = 0.0;
    data_buffer_[5] = 0.0;
    dirty_ = true;
}

void DataModel::set_connection_state(ConnectionState state) {
    switch (state) {
    case ConnectionState::DISCONNECTED:
        connection_state_string_ = "DISCONNECTED";
        break;
    case ConnectionState::CONNECTING:
        connection_state_string_ = "CONNECTING";
        break;
    case ConnectionState::CONNECTED:
        connection_state_string_ = "CONNECTED";
        break;
    case ConnectionState::NEEDS_UPDATE:
        connection_state_string_ = "NEEDS_UPDATE";
        break;
    case ConnectionState::UNKNOWN:
        connection_state_string_ = "UNKNOWN";
        break;
    }
    dirty_ = true;
}

void DataModel::set_version(const std::string &version) {
    version_string_ = version;
    dirty_ = true;
}

void DataModel::set_accel_data(const std::shared_ptr<MuseDataPacket> &p) {
    mDataAccel[0] = p->get_accelerometer_value(Accelerometer::X);
	mDataAccel[1] = p->get_accelerometer_value(Accelerometer::Y);
	mDataAccel[2] = p->get_accelerometer_value(Accelerometer::Z);
	mDataAccel[3] = 0.0;
	mDataAccel[4] = 0.0;
	mDataAccel[5] = 0.0;

}

void DataModel::set_battery_data(const std::shared_ptr<MuseDataPacket> &p) {
    mDataBattery[0] = p->get_battery_value(Battery::CHARGE_PERCENTAGE_REMAINING);
	mDataBattery[1] = p->get_battery_value(Battery::MILLIVOLTS);
	mDataBattery[2] = p->get_battery_value(Battery::TEMPERATURE_CELSIUS);
	mDataBattery[3] = 0.0;
	mDataBattery[4] = 0.0;
	mDataBattery[5] = 0.0;
	mDirtyBattery = true;
}

void DataModel::set_drl_ref_data(const std::shared_ptr<MuseDataPacket> &p) {
    mDataDrlRef[0] = p->get_drl_ref_value(DrlRef::DRL);
	mDataDrlRef[1] = p->get_drl_ref_value(DrlRef::REF);
	mDataDrlRef[2] = 0.0;
	mDataDrlRef[3] = 0.0;
	mDataDrlRef[4] = 0.0;
	mDataDrlRef[5] = 0.0;
	mDirtyDrlRef = true;
}

void DataModel::set_eeg_data(const std::shared_ptr<MuseDataPacket> &p ) {
    mDataEgg[0] = p->get_eeg_channel_value(Eeg::EEG1);
	mDataEgg[1] = p->get_eeg_channel_value(Eeg::EEG2);
	mDataEgg[2] = p->get_eeg_channel_value(Eeg::EEG3);
	mDataEgg[3] = p->get_eeg_channel_value(Eeg::EEG4);
	mDataEgg[4] = p->get_eeg_channel_value(Eeg::AUX_LEFT);
	mDataEgg[5] = p->get_eeg_channel_value(Eeg::AUX_RIGHT);
	mDirtyEgg = true;
}

void DataModel::setEggData(const std::shared_ptr<interaxon::bridge::MuseDataPacket> &p, double * buffer, bool *dirtyFlag)
{
	buffer[0] = p->get_eeg_channel_value(Eeg::EEG1);
	buffer[1] = p->get_eeg_channel_value(Eeg::EEG2);
	buffer[2] = p->get_eeg_channel_value(Eeg::EEG3);
	buffer[3] = p->get_eeg_channel_value(Eeg::EEG4);
	buffer[4] = p->get_eeg_channel_value(Eeg::AUX_LEFT);
	buffer[5] = p->get_eeg_channel_value(Eeg::AUX_RIGHT);
	*dirtyFlag = true;
};


void DataModel::set_gyro_data(const std::shared_ptr<MuseDataPacket> &p) {
    mDataGyro[0] = p->get_gyro_value(Gyro::X);
	mDataGyro[1] = p->get_gyro_value(Gyro::Y);
	mDataGyro[2] = p->get_gyro_value(Gyro::Z);
	mDataGyro[3] = 0.0;
	mDataGyro[4] = 0.0;
	mDataGyro[5] = 0.0;
	mDirtyGyro = true;
}
/*
void getBufferBattery(double * buffer) const
{
}
*/