﻿//
// MainPage.xaml.h
// Declaration of the MainPage class.
//


/*****************************
Modificaciones por Xumerio
- Se modifica la clase DataModel para guardar 6 buffers para cada tipo
- Se modifica la GUI principal, no habrá combo box y se agrean checkboxes

*******************************/

#pragma once

#include "MainPage.g.h"
#include "DataModel.h"
#include <iostream>
#include "muse.h"
#include <winsock2.h>
#include <ws2tcpip.h>

using namespace interaxon::bridge;

namespace GettingData
{
	class MuseListenerWin;
	class DataListener;
	class ConnectionListener;

	/// <summary>
	/// MainPage contains the UI of the example application and manages the
	/// connection to the Muse headband.  Data is received from the headband
	/// via a listener callback.  Received data is stored in the DataModel
	/// until the next UI update.
	/// </summary>
	public ref class MainPage sealed
	{
	public:

		/// Constructs a MainPage
		MainPage();

	private:

		//Parte OSC
		//scrutura para soportar direccion y puert
		struct direccion {
			std::string ip{ "127.0.0.1" };
			int			port{ 7000 };
		};
		//contenedor de direcciones a las que se le enviarán los datos en formato PSC
		std::vector<direccion> mDirecciones;

		int numSensor = 2;
		//Funciones para enviar tipos de datos diferentes por OSC
		void sendOsc(std::string address, float value);					// sin sensor
		void sendOsc(std::string address, int sensor, float value);		// con sensor
		void sendOscFloat(std::string tag, float value);				// sin sensor
		void sendOscFloat(std::string tag, int sensor, float value);	// con sensor
		void sendOscInt(std::string address, std::string tag, int value);
		void sendOscInt(std::string address, std::string tag, int sensor, int value);
		void sendOscFloatVector(std::string address, std::string tag, const std::vector<double> values);
		void sendOscFloatVector(std::string address, std::string tag, int sensor, const std::vector<double> values);
		//Función para validación de dirección IP, regresa false si la cadena de texto no está en formato IPv4 válido
		bool validateIpAddress(const std::string &ipAddress);

		//Manda el promedio del arreglo buffer
		//Todos los arreglos buffer se consideran de 6 elementos
		void sendMeanBuffer(double * buffer, std::string tag);
		//Calcula el promedio del buffers
		double getMean(double * buffer);

		void send_this_Checked();

		void registerListener();

		//////////////////////////////////////////////////////
		/// Bluetooth functions

		/// Creates an asynchronous task to check if there are
		/// any Bluetooth radios that are currently turned on.
		/// The result is accessible from is_bluetooth_enabled.
		void check_bluetooth_enabled();

		/// Returns the last result from the call to check_bluetooth_enabled().
		/// Returns true if a Bluetooth radio is on, false if all Bluetooth
		/// radios are off.
		bool is_bluetooth_enabled();


		//////////////////////////////////////////////////////
		/// Listener callback functions.
		///

		/// muse_list_changed is called whenever a new headband is detected that
		/// can be connected to or an existing headband is detected as no longer
		/// available.  You must be "listening" in order to receive these callbacks.
		/// To start listening, call MuseManager::start_listening();
		///
		/// Once you have received the callback, you can get the available
		/// headbands by calling MuseManager::get_muses();
		void muse_list_changed();

		/// You receive a connection packet whenever the state of a headband changes
		/// between DISCONNECTED, CONNECTING and CONNECTED.
		///
		/// \param packet Contains the prior and new connection states.
		/// \param muse   The muse that sent the connection packet.
		void receive_connection_packet(const MuseConnectionPacket & packet, const std::shared_ptr<Muse> & muse);

		/// Most data from the headband, including raw EEG values, is received as
		/// MuseDataPackets.  While this example only illustrates one packet type
		/// at a time, it is possible to receive multiple data types at the same time.
		/// You can use MuseDataPacket::packet_type();
		///
		/// \param packet The data packet.
		/// \param muse   The muse that sent the data packet.
		void receive_muse_data_packet(const std::shared_ptr<MuseDataPacket> & packet, const std::shared_ptr<Muse> & muse);

		/// Artifacts are boolean values that are derived from the EEG data
		/// such as eye blinks or clenching of the jaw.
		///
		/// \param packet The artifact packet.
		/// \param muse   The muse that sent the artifact packet.
		void receive_muse_artifact_packet(const MuseArtifactPacket & packet, const std::shared_ptr<Muse> & muse);


		//////////////////////////////////////////////////////
		///  UI Callbacks

		/// Called when the user presses the "Refresh" button.
		/// This calls MuseManager::start_listening to start receiving muse_list_changed
		/// callbacks.
		void refresh_button_clicked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

		/// Called when the user presses the "Connect" button.
		/// This will initiate a connection to the Muse headband selected in the
		/// top drop down.
		void connect_button_clicked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

		/// Called when the user presses the "Disconnect" button.
		/// This will disconnect from the Muse headband that is currently connected.
		void disconnect_button_clicked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

		/// Called when the user presses the "Pause/Resume" button
		/// This will toggle between pause and resume data transmission.
		void pause_resume_clicked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);




		//////////////////////////////////////////////////////
		///  Helper functions for the UI

		/// EEG Data is received at a much higher rate (220 or 256Hz) than it is
		/// necessary to update the UI.  This function will ask for a UI update
		/// at a more reasonable rate (60Hz).
		void queue_ui_update();

		/// Handles a single update of the screen by transferring the data from
		/// the DataModel to the UI elements.
		///
		/// Called from queue_ui_update on the Core Dispatcher.
		void update_ui();


		/// Updates the combobox at the top of the screen with the list of
		/// available Muse headbands.  Called when a muse_list_changed callback
		/// is received.
		void update_muse_list();

		/// Finds the Muse with the specified name in the list of Muses returned
		/// from MuseManager.
		std::shared_ptr<Muse> get_muse(Platform::String^ name);

		/// Unregisters the current MuseDataListener and registers a listener for
		/// the new data type.
		///
		/// \param type The MuseDataPacketType you want to display.
		void change_data_type(MuseDataPacketType type);


		/// Formats a double value to a String with the desired number of
		/// decimal places.
		Platform::String^ formatData(double data) const;




		//////////////////////////////////////////////////////
		///  Variables

		// A reference to the MuseManager instance.
		std::shared_ptr<interaxon::bridge::MuseManagerWindows> manager_;

		/// The individual listener interfaces in LibMuse are abstract classes.
		/// The following classes are defined at the end of the file.  Each
		/// inner class implements a different interface and forwards the
		/// information received back to this MainPage object.
		friend class ConnectionListener;
		friend class MuseListenerWin;
		friend class DataListener;

		std::shared_ptr<MuseListenerWin> muse_listener_;
		std::shared_ptr<ConnectionListener> connection_listener_;
		std::shared_ptr<DataListener> data_listener_;

		/// A reference to the Muse object that we are currently connected to.
		/// This is useful so we know which Muse to disconnect.
		std::shared_ptr<Muse> my_muse_;

		/// The current type of data that we are listening to.  This is set
		/// from the combobox in the middle of the screen.
		MuseDataPacketType current_data_type_;

		/// The Data Model object that is used to collect and store the data
		/// we receive.
		DataModel model_;

		/// The last result returned from the check_bluetooth_enabled function.
		std::atomic_bool is_bluetooth_enabled_;

		/// A map for getting the MuseDataPacketType from the name in the
		/// data type combobox.
		Platform::Collections::Map<Platform::String^, int> name_to_type_map_;

		//Platform::Collections::Map<Platform::String^, int> name_to_type_map_;


		/// Toogle for pause/resume data transmission.
		bool enable_data_;
		void Button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void addIP_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void Button_Click_1(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	};


	//////////////////////////////////////////////////////
	///  Listener Implementation Classes

	class MuseListenerWin : public interaxon::bridge::MuseListener {
	public:
		MuseListenerWin(MainPage^ mp) : main_page_(mp) {}
		void muse_list_changed() override {
			main_page_->muse_list_changed();
		}
	private:
		MainPage^ main_page_;
	};

	class ConnectionListener : public interaxon::bridge::MuseConnectionListener {
	public:
		ConnectionListener(MainPage^ mp) : main_page_(mp) {}
		void receive_muse_connection_packet(const MuseConnectionPacket & packet, const std::shared_ptr<Muse> & muse) override {
			main_page_->receive_connection_packet(packet, muse);
		}
	private:
		MainPage^ main_page_;
	};

	class DataListener : public MuseDataListener {
	public:
		DataListener(MainPage^ mp) : main_page_(mp) {}
		void receive_muse_data_packet(const std::shared_ptr<MuseDataPacket> & packet, const std::shared_ptr<Muse> & muse) override {
			main_page_->receive_muse_data_packet(packet, muse);
		}

		void receive_muse_artifact_packet(const MuseArtifactPacket & packet, const std::shared_ptr<Muse> & muse) override {
			main_page_->receive_muse_artifact_packet(packet, muse);
		}
	private:
		MainPage^ main_page_;
	};
}
