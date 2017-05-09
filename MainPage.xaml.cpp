//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"

#include <sstream>
#include <iomanip>

#include <ppltasks.h>

#include "muse.h"
////////////////////////////////////////////////////////
#include "osc/OscOutboundPacketStream.h"
#include "ip/UdpSocket.h"

//#include <QInputDialog>
/////////////////////////////////////////////////

using namespace GettingData;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::Devices::Radios;

using namespace interaxon;
using namespace concurrency;



// Refresh rate for the data display on the screen (Hz)
#define REFRESH_RATE 60
// Decimal precision for displayed data
#define DATA_PRECISION 4

MainPage::MainPage() :
	current_data_type_(MuseDataPacketType::ACCELEROMETER),
	my_muse_(nullptr),
	enable_data_(true)
{
	InitializeComponent();


	queue_ui_update();

	manager_ = MuseManagerWindows::getInstance();
	muse_listener_ = std::make_shared<GettingData::MuseListenerWin>(this);
	connection_listener_ = std::make_shared<GettingData::ConnectionListener>(this);
	data_listener_ = std::make_shared<GettingData::DataListener>(this);
	manager_->set_muse_listener(muse_listener_);
	is_bluetooth_enabled_.store(false);
	check_bluetooth_enabled();
	
	
}




//******************************************
#define ADDRESS "127.0.0.1"
#define PORT 7000

#define OUTPUT_BUFFER_SIZE 1024
void MainPage::sendOsc(std::string address, float value)
/* void MainPage::sendOsc(std::string address, float value) */
{
	UdpTransmitSocket transmitSocket(IpEndpointName(ADDRESS, PORT));

	char buffer[OUTPUT_BUFFER_SIZE];
	osc::OutboundPacketStream p(buffer, OUTPUT_BUFFER_SIZE);

	p << osc::BeginBundleImmediate
		<< osc::BeginMessage(address.c_str())
		<< value << osc::EndMessage
		<< osc::EndBundle;

	transmitSocket.Send(p.Data(), p.Size());
}
void MainPage::sendOscFloat(std::string tag, float value)
{
	for (auto &dire : mDirecciones) {
		UdpTransmitSocket transmitSocket(IpEndpointName(dire.ip.c_str(), dire.port));

		char buffer[OUTPUT_BUFFER_SIZE];
		osc::OutboundPacketStream p(buffer, OUTPUT_BUFFER_SIZE);

		p << osc::BeginBundleImmediate
			<< osc::BeginMessage(tag.c_str())
			<< value << osc::EndMessage
			<< osc::EndBundle;

		transmitSocket.Send(p.Data(), p.Size());
	}
}
void MainPage::sendOscInt(std::string address, std::string tag, int value)
{
	for (auto &dire : mDirecciones) {
		UdpTransmitSocket transmitSocket(IpEndpointName(dire.ip.c_str(), dire.port));

		char buffer[OUTPUT_BUFFER_SIZE];
		osc::OutboundPacketStream p(buffer, OUTPUT_BUFFER_SIZE);

		p << osc::BeginBundleImmediate
			<< osc::BeginMessage(tag.c_str())
			<< value << osc::EndMessage
			<< osc::EndBundle;

		transmitSocket.Send(p.Data(), p.Size());
	}
}

void MainPage::sendOscFloatVector(std::string address, std::string tag, const std::vector<double> values)
{
	for (auto &dire : mDirecciones) {
		UdpTransmitSocket transmitSocket(IpEndpointName(dire.ip.c_str(), dire.port));

		char buffer[OUTPUT_BUFFER_SIZE];
		osc::OutboundPacketStream p(buffer, OUTPUT_BUFFER_SIZE);


		std::stringstream ss;
		

		p << osc::BeginBundleImmediate
			<< osc::BeginMessage(tag.c_str());
		
		for (auto value : values) {
			p << value;
		}

		p << osc::EndMessage << osc::EndBundle;

		transmitSocket.Send(p.Data(), p.Size());
	}
}

//Para mandar el promedio de un buffer
void MainPage::sendMeanBuffer(double *buffer, std::string tag)
{
	double mean = 0;
	for (int i = 0; i < 6; i++) {
		mean += buffer[i];
	}
	mean /= 6;

	sendOscFloat(tag, mean);
}
double MainPage::getMean(double * buffer)
{
	double mean = 0;
	for (int i = 0; i < 6; i++) {
		mean += buffer[i];
	}
	mean /= 6;
	return mean;
}
// Prueba
void MainPage::send_this_Checked() {

}

void GettingData::MainPage::registerListener()
{
	my_muse_->register_data_listener(data_listener_, MuseDataPacketType::ACCELEROMETER);
	my_muse_->register_data_listener(data_listener_, MuseDataPacketType::GYRO);
	my_muse_->register_data_listener(data_listener_, MuseDataPacketType::EEG);
	//my_muse_->register_data_listener(data_listener_, MuseDataPacketType::DROPPED_ACCELEROMETER);
	//my_muse_->register_data_listener(data_listener_, MuseDataPacketType::DROPPED_EEG);
	//my_muse_->register_data_listener(data_listener_, MuseDataPacketType::QUANTIZATION);
	my_muse_->register_data_listener(data_listener_, MuseDataPacketType::BATTERY);
	//my_muse_->register_data_listener(data_listener_, MuseDataPacketType::ALPHA_ABSOLUTE);
	//my_muse_->register_data_listener(data_listener_, MuseDataPacketType::BETA_ABSOLUTE);
	//my_muse_->register_data_listener(data_listener_, MuseDataPacketType::DELTA_ABSOLUTE);
	//my_muse_->register_data_listener(data_listener_, MuseDataPacketType::THETA_ABSOLUTE);
	//my_muse_->register_data_listener(data_listener_, MuseDataPacketType::GAMMA_ABSOLUTE);
	my_muse_->register_data_listener(data_listener_, MuseDataPacketType::ALPHA_RELATIVE);
	my_muse_->register_data_listener(data_listener_, MuseDataPacketType::BETA_RELATIVE);
	my_muse_->register_data_listener(data_listener_, MuseDataPacketType::DELTA_RELATIVE);
	my_muse_->register_data_listener(data_listener_, MuseDataPacketType::THETA_RELATIVE);
	my_muse_->register_data_listener(data_listener_, MuseDataPacketType::GAMMA_RELATIVE);
	//my_muse_->register_data_listener(data_listener_, MuseDataPacketType::ALPHA_SCORE);
	//my_muse_->register_data_listener(data_listener_, MuseDataPacketType::BETA_SCORE);
	//my_muse_->register_data_listener(data_listener_, MuseDataPacketType::DELTA_SCORE);
	//my_muse_->register_data_listener(data_listener_, MuseDataPacketType::THETA_SCORE);
	//my_muse_->register_data_listener(data_listener_, MuseDataPacketType::GAMMA_SCORE);
	my_muse_->register_data_listener(data_listener_, MuseDataPacketType::IS_GOOD);
	//my_muse_->register_data_listener(data_listener_, MuseDataPacketType::HSI);
	//my_muse_->register_data_listener(data_listener_, MuseDataPacketType::HSI_PRECISION);
	
	my_muse_->register_data_listener(data_listener_, MuseDataPacketType::ARTIFACTS);
}



void MainPage::check_bluetooth_enabled() {
	// This task is async and will update the member variable when it is run.
	create_task(Radio::GetRadiosAsync()).then([this](IVectorView<Radio^>^ radios) {
		for (auto r : radios) {
			if (r->Kind == RadioKind::Bluetooth) {
				if (r->State == RadioState::On) {
					is_bluetooth_enabled_.store(true);
				}
				else {
					is_bluetooth_enabled_.store(false);
				}
				break;
			}
		}
	});
}

bool MainPage::is_bluetooth_enabled() {
	// Must call check_bluetooth_enabled first to run
	// an async to check if bluetooth radio is on.
	return is_bluetooth_enabled_.load();
}

// Muse callback methods
void MainPage::muse_list_changed() {
	OutputDebugString(L"MainPage::muse_list_changed\n");
	update_muse_list();
}

void MainPage::receive_connection_packet(const MuseConnectionPacket & packet, const std::shared_ptr<Muse> & muse) {
	model_.set_connection_state(packet.current_connection_state);
	OutputDebugString(L"MainPage::receive_connection_packet\n");

	// The Muse version is only available in the connected state.
	if (packet.current_connection_state == ConnectionState::CONNECTED) {
		auto version = my_muse_->get_muse_version();
		model_.set_version(version->get_firmware_version());
		registerListener();
	}
	else {
		model_.set_version("Unknown");
	}

}
//REcibe los paquetes
void MainPage::receive_muse_data_packet(const std::shared_ptr<MuseDataPacket> & packet, const std::shared_ptr<Muse> & muse) {
	model_.set_values(packet);
	//OutputDebugString(L"MainPage::receive_muse_data_packet\n");
}
//Recibe los artifacts, parece que son eventos menos frecuente, al recibir envio y no meto al data model.
void MainPage::receive_muse_artifact_packet(const MuseArtifactPacket & packet, const std::shared_ptr<Muse> & muse) {
	model_.set_values(packet);
	OutputDebugString(L"MainPage::receive_artifact_packet\n");
	
	
}

void MainPage::refresh_button_clicked(Platform::Object^ sender,
	Windows::UI::Xaml::RoutedEventArgs^ e)
{
	manager_->stop_listening();
	manager_->start_listening();
}

void MainPage::connect_button_clicked(Platform::Object^ sender,
	Windows::UI::Xaml::RoutedEventArgs^ e)
{
	auto selected_muse = static_cast<Platform::String^>(muse_list_combobox->SelectedItem);
	my_muse_ = get_muse(selected_muse);

	if (nullptr != my_muse_) {
		// Stop listening after selecting a muse to connect to
		manager_->stop_listening();
		model_.clear();
		my_muse_->register_connection_listener(connection_listener_);
		my_muse_->register_data_listener(data_listener_, current_data_type_);
		my_muse_->run_asynchronously();
	}
}
void MainPage::disconnect_button_clicked(Platform::Object^ sender,
	Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (my_muse_ != nullptr) {
		my_muse_->disconnect();
	}
}

void MainPage::pause_resume_clicked(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{
	if (my_muse_ != nullptr) {
		enable_data_ = !enable_data_;
		my_muse_->enable_data_transmission(enable_data_);
	}
}


void MainPage::queue_ui_update() {
	create_task([this]() {
		WaitForSingleObjectEx(GetCurrentThread(), 1000 / REFRESH_RATE, false);
	}).then([this]() {
		Dispatcher->RunAsync(
			Windows::UI::Core::CoreDispatcherPriority::Normal,
			ref new Windows::UI::Core::DispatchedHandler([this]()
		{
			update_ui();
		})
		);
	});
}

// Call only from the UI thread.
void MainPage::update_ui() {

	/*
	Platform::IBox<bool>^ iboxBool;
	iboxBool = enviaAlpha->IsChecked;
	if (iboxBool->Value) {
	}
	*/
	//El horror, se requiere para comprobar si el checkbox está listo
	Platform::IBox<bool>^ iboxBool;
	//Si se ha recibido cualquier tipo de datos proceder
	if (model_.is_dirty()) {
		connection_status->Text = Convert::to_platform_string(model_.get_connection_state());
		version->Text = Convert::to_platform_string(model_.get_version());


		
		//Si ha recibido nuevos valores para Alpha relative
		if (model_.isDirtyEggAlpha()) {
			double buffer[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
			model_.getBufferEggAplha(buffer);
			double mean = getMean(buffer);
			alphaPromedio->Text = "" + mean;
			iboxBool = enviaAlpha->IsChecked;
			if (iboxBool->Value) {

				sendOscFloat("/alpha", mean);
				//sendMeanBuffer(buffer, "/alpha");	
			}
			model_.clearDirtyEggAplha();
		}

		//Si ha recibido nuevos valores para Beta relative
		if (model_.isDirtyEggBeta()) {
			double buffer[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
			model_.getBufferEggBeta(buffer);
			double mean = getMean(buffer);
			betaPromedio->Text = "" + mean;
			iboxBool = enviaBeta->IsChecked;
			if (iboxBool->Value) {
				sendOscFloat("/beta", mean);
			}
			model_.clearDirtyEggBeta();
		}

		//Si ha recibido nuevos valores para Delta relative
		if (model_.isDirtyEggDelta() == true) {
			double buffer[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
			model_.getBufferEggDelta(buffer);
			double mean = getMean(buffer);
			deltaPromedio->Text = "" + mean;
			iboxBool = enviaDelta->IsChecked;
			if (iboxBool->Value) {
				sendOscFloat("/delta", mean);
			}
			model_.clearDirtyEggDelta();
		}

		//Si ha recibido nuevos valores para Theta relative
		if (model_.isDirtyEggTheta()) {
			double buffer[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
			model_.getBufferEggTheta(buffer);
			double mean = getMean(buffer);
			thetaPromedio->Text = "" + mean;
			iboxBool = enviaTheta->IsChecked;
			if (iboxBool->Value) {
				sendOscFloat("/theta", mean);
			}
			model_.clearDirtyEggTheta();
		}

		//Si ha recibido nuevos valores para Theta relative
		if (model_.isDirtyEggGamma()) {
			double buffer[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
			model_.getBufferEggGamma(buffer);
			double mean = getMean(buffer);
			gammaPromedio->Text = "" + mean;
			iboxBool = enviaGamma->IsChecked;
			if (iboxBool->Value) {
				sendOscFloat("/gamma", mean);
			}
			model_.clearDirtyEggGamma();
		}


		//Mandar los datos de Battery
		if (model_.isDirtyBattery()) {
			double buffer[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
			model_.getBufferBattery(buffer);
			battery->Text = "" + buffer[0];
			iboxBool = Battery->IsChecked;
			if (iboxBool->Value) {
				sendOscFloat("/battery", (float)buffer[0]);
			}

			model_.clearDirtyBattery();
		}

		//Mandar los datos del acelerómetro
		if (model_.isDirtyAccel()) {
			double buffer[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
			model_.getBufferAccel(buffer);
			accX->Text = "" + buffer[0];
			accY->Text = "" + buffer[1];
			accZ->Text = "" + buffer[2];
			iboxBool = enviaAccelerometer->IsChecked;
			if (iboxBool->Value) {
				std::vector<double> dats{ buffer[0], buffer[1], buffer[2] };
				sendOscFloatVector("", "/accelerometer", dats);
			}

			model_.clearDirtyAccel();
		}

		//Mandar los datos del gyro
		if (model_.isDirtyGyro() == true) {
			double buffer[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
			model_.getBufferGyro(buffer);
			gyroX->Text = "" + buffer[0];
			gyroY->Text = "" + buffer[1];
			gyroZ->Text = "" + buffer[2];
			iboxBool = enviaGyro->IsChecked;
			if (iboxBool->Value) {
				std::vector<double> dats{ buffer[0], buffer[1], buffer[2] };
				sendOscFloatVector("", "/gyro", dats);
			}

			model_.clearDirtyGyro();
		}


		//Mandar los datos EGG
		//OH chinga!
		if (model_.isDirtyEgg() == true) {
			double buffer[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
			model_.getBufferEggAplha(buffer);
			EEG1->Text = "" + buffer[0];
			EEG2->Text = "" + buffer[1];
			EEG3->Text = "" + buffer[2];
			EEG4->Text = "" + buffer[3];
			auxLeft->Text = "" + buffer[4];
			auxRight->Text = "" + buffer[5];

			iboxBool = enviaIsGood->IsChecked;
			if (iboxBool->Value) {
				std::vector<double> dats{ buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5] };
				sendOscFloatVector("", "/eeg", dats);

			}
		}

		if (model_.isDirtyArtifacts() == true) {
			double buffer[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
			model_.getBufferActifacts(buffer);

			artifactHead->Text	= "" + buffer[0];
			artifactBlink->Text = "" + buffer[1];
			artifactJaw->Text	= "" + buffer[2];

			//return;
			Platform::IBox<bool>^ iboxBool;
			iboxBool = enviaArtifacts->IsChecked;
			if (iboxBool->Value) {
				std::vector<double> dats{ buffer[0], buffer[1], buffer[2] };
				sendOscFloatVector("", "/artifacts", dats);

			}
			model_.clearDirtyArtifacts();
		}

		model_.clear();
	}
	queue_ui_update();
}

void MainPage::update_muse_list() {
	auto muses = manager_->get_muses();
	muse_list_combobox->Items->Clear();
	for (auto m : muses) {
		auto name = Convert::to_platform_string(m->get_name());
		muse_list_combobox->Items->Append(name);
	}
	if (muse_list_combobox->Items->Size) {
		muse_list_combobox->SelectedIndex = 0;
	}
}

std::shared_ptr<Muse> MainPage::get_muse(Platform::String^ desired_name) {
	auto muses = manager_->get_muses();
	for (auto m : muses) {
		auto name = Convert::to_platform_string(m->get_name());
		if (name->Equals(desired_name)) {
			return m;
		}
	}
	return nullptr;
}

void MainPage::change_data_type(MuseDataPacketType type) {
	if (my_muse_ != nullptr) {
		my_muse_->unregister_data_listener(data_listener_, current_data_type_);
		my_muse_->register_data_listener(data_listener_, type);
	}
	current_data_type_ = type;
}




Platform::String^ MainPage::formatData(double data) const
{
	std::stringstream ss;
	ss << std::fixed << std::setprecision(DATA_PRECISION) << data;
	return Convert::to_platform_string(ss.str());
}


void GettingData::MainPage::Button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (listaDirecciones->Items->Size < 1) {
		return;
	}

	if (mDirecciones.size() != listaDirecciones->Items->Size) {
		return;
	}

	if (listaDirecciones->SelectedIndex >= 0) {

		mDirecciones.erase(mDirecciones.begin() + listaDirecciones->SelectedIndex);
		listaDirecciones->Items->RemoveAt(listaDirecciones->SelectedIndex);
	}
}



void GettingData::MainPage::addIP_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{

	if (nuevaIp->Text->ToString() == "" ||  nuevoPuerto->Text->ToString() == "")
		return;
	Platform::String^ fooRT = nuevaIp->Text->ToString();
	std::wstring fooW(fooRT->Begin());
	std::string fooA(fooW.begin(), fooW.end());

	nuevaIp;
	if ( !validateIpAddress( fooA ) )
		return;

	Platform::String^ compuesta = nuevaIp->Text->ToString() + ":" + nuevoPuerto->Text->ToString();

	listaDirecciones->Items->Append(compuesta);

	

	direccion dire;
	dire.ip = fooA;
	dire.port = _wtoi(nuevoPuerto->Text->ToString()->Data());

	

	mDirecciones.push_back(dire);


}


void GettingData::MainPage::Button_Click_1(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	model_.setAllDirty(true);
	
	
	Platform::IBox<bool>^ iboxBool;
	iboxBool = enviaArtifacts->IsChecked;

	artifactHead->Text = "" + 2.0;
	artifactBlink->Text = "" + 2.0;
	artifactJaw->Text = "" + 2.0;
	if (iboxBool->Value) {
		std::vector<double> dats{ 1.0, 1.0, 1.0 };
		sendOscFloatVector("", "/artifacts", dats);

	}


}

bool GettingData::MainPage::validateIpAddress(const std::string &ipAddress)
{
	struct sockaddr_in sa;
	int result = inet_pton(AF_INET, ipAddress.c_str(), &(sa.sin_addr));
	return result != 0;
}


void GettingData::MainPage::enviaBeta_Checked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{

}
