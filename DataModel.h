#pragma once

namespace interaxon {

    namespace bridge {
        struct MuseArtifactPacket;
        class MuseDataPacket;
        enum class ConnectionState;
    }
}

/// <summary>
/// DataModel is used to store the information received from the Muse
/// headband until the UI can be updated to reflect it.
/// </summary>
class DataModel {

public:

    DataModel();
    virtual ~DataModel();

    /// Returns true if the model is "dirty", ie. new data has been received
    /// since the last time the UI was updated.
    bool is_dirty() const;

	bool isDirtyAccel() const { return mDirtyAccel; };
	bool isDirtyBattery() const { return mDirtyBattery; };
	bool isDirtyEgg() const { return mDirtyEgg; };
	bool isDirtyDrlRef() const { return mDirtyDrlRef; };
	bool isDirtyGyro() const { return mDirtyGyro; };
	bool isDirtyEggAlpha() const { return mDirtyEggAlpha; };
	bool isDirtyEggBeta() const { return mDirtyEggBeta; };
	bool isDirtyEggDelta() const { return mDirtyEggDelta; };
	bool isDirtyEggTheta() const { return mDirtyEggTheta; };
	bool isDirtyEggGamma() const { return mDirtyEggGamma; };

    /// Called by the UI update function to indicate that the UI has been
    /// updated with all of the data contained in the model.
    void clear_dirty_flag();

	//Estas variables dirty son para avisar si hay datos nuevos que se deban enviar
	//Uno para cada uno de los campos que necesitamos.
	void clearDirtyAccel() { mDirtyAccel = false; };
	void clearDirtyBattery() { mDirtyBattery = false; };
	void clearDirtyDrlRef() { mDirtyDrlRef = false; };
	void clearDirtyGyro() { mDirtyGyro = false; };
	void clearDirtyEggAplha() { mDirtyEggAlpha = false; };
	void clearDirtyEggBeta() { mDirtyEggBeta = false; };
	void clearDirtyEggDelta() { mDirtyEggDelta = false; };
	void clearDirtyEggTheta() { mDirtyEggTheta = false; };
	void clearDirtyEggGamma() { mDirtyEggGamma = false; };

	void setAllDirty(bool dirty);

    /// Erases all data stored in the model.  More heavyweight than clear_dirty_flag
    void clear();

    /// Copies the values of the latest received data packet to the
    /// provided double array.  The double array should be large enough to
    /// store 6 values.
    void get_buffer(double *buffer) const;
	//Recibir los buffers que varios tipos para luego copiarlos
	void getBufferAccel(double *buffer) const;
	void getBufferBattery(double *buffer) const;
	void getBufferDrlRef(double *buffer) const;
	void getBufferGyro(double *buffer) const;
	void getBufferEgg(double *buffer) const;
	void getBufferEggAplha(double *buffer) const;
	void getBufferEggBeta(double *buffer) const;
	void getBufferEggDelta(double *buffer) const;
	void getBufferEggTheta(double *buffer) const;
	void getBufferEggGamma(double *buffer) const;

    /// Returns the current connection state of the Muse headband.
    const std::string &get_connection_state() const;

    /// Returns the current version of the connected Muse headband.
    /// This value is not available if the headband is not connected.
    const std::string &get_version() const;

    /// Stores the appropriate information from the Muse information packets
    /// in the private variables of this class.
    void set_values(const std::shared_ptr<interaxon::bridge::MuseDataPacket> &p);
    void set_values(const interaxon::bridge::MuseArtifactPacket &p);
    void set_connection_state(interaxon::bridge::ConnectionState state);
    void set_version(const std::string &version);

private:

    /// Utility functions for mapping the values in the MuseDataPacket to
    /// the double array data_buffer_
    void set_accel_data(const std::shared_ptr<interaxon::bridge::MuseDataPacket> &p);
    void set_battery_data(const std::shared_ptr<interaxon::bridge::MuseDataPacket> &p);
    void set_drl_ref_data(const std::shared_ptr<interaxon::bridge::MuseDataPacket> &p);
    void set_eeg_data(const std::shared_ptr<interaxon::bridge::MuseDataPacket> &p);
    void set_gyro_data(const std::shared_ptr<interaxon::bridge::MuseDataPacket> &p);

	void setEggData(const std::shared_ptr<interaxon::bridge::MuseDataPacket> &p, double * buffer, bool *dirtyFlag);
    /// Since we are only interested in one data packet type at a time, we
    /// reuse a single double array to store the data and map the data from
    /// the data packet to a structure the UI expects.
    double *data_buffer_;

	//Buffers para los diferentes tipos de datos
	double * mDataAccel,
		*mDataBattery,
		*mDataEgg,
		*mDataGyro,
		*mDataDrlRef,
		*mDataEggAlpha,
		*mDataEggBeta,
		*mDataEggDelta,
		*mDataEggTheta,
		*mDataEggGamma;


    /// The String that represents the latest received connection state
    /// information.
    std::string connection_state_string_;

    /// The version of firmware that is running on the headband.
    std::string version_string_;

    /// The dirty state of the model.
    bool dirty_;

	bool mDirtyAccel,
		mDirtyBattery,
		mDirtyEgg,
		mDirtyGyro,
		mDirtyDrlRef,
		mDirtyEggAlpha,
		mDirtyEggBeta,
		mDirtyEggDelta,
		mDirtyEggTheta,
		mDirtyEggGamma
		;

};