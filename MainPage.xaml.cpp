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
    init_data_type_combobox();

    queue_ui_update();

    manager_ = MuseManagerWindows::getInstance();
    muse_listener_ = std::make_shared<GettingData::MuseListenerWin>(this);
    connection_listener_ = std::make_shared<GettingData::ConnectionListener>(this);
    data_listener_ = std::make_shared<GettingData::DataListener>(this);
    manager_->set_muse_listener(muse_listener_);
    is_bluetooth_enabled_.store(false);
    check_bluetooth_enabled();
}

void MainPage::init_data_type_combobox() {
    add_type("ACCELEROMETER", MuseDataPacketType::ACCELEROMETER);
    add_type("GYRO", MuseDataPacketType::GYRO);
    add_type("EEG", MuseDataPacketType::EEG);
    add_type("DROPPED_ACCELEROMETER", MuseDataPacketType::DROPPED_ACCELEROMETER);
    add_type("DROPPED_EEG", MuseDataPacketType::DROPPED_EEG);
    add_type("QUANTIZATION", MuseDataPacketType::QUANTIZATION);
    add_type("BATTERY", MuseDataPacketType::BATTERY);
    add_type("DRL_REF", MuseDataPacketType::DRL_REF);
    add_type("ALPHA_ABSOLUTE", MuseDataPacketType::ALPHA_ABSOLUTE);
    add_type("BETA_ABSOLUTE", MuseDataPacketType::BETA_ABSOLUTE);
    add_type("DELTA_ABSOLUTE", MuseDataPacketType::DELTA_ABSOLUTE);
    add_type("THETA_ABSOLUTE", MuseDataPacketType::THETA_ABSOLUTE);
    add_type("GAMMA_ABSOLUTE", MuseDataPacketType::GAMMA_ABSOLUTE);
    add_type("ALPHA_RELATIVE", MuseDataPacketType::ALPHA_RELATIVE);
    add_type("BETA_RELATIVE", MuseDataPacketType::BETA_RELATIVE);
    add_type("DELTA_RELATIVE", MuseDataPacketType::DELTA_RELATIVE);
    add_type("THETA_RELATIVE", MuseDataPacketType::THETA_RELATIVE);
    add_type("GAMMA_RELATIVE", MuseDataPacketType::GAMMA_RELATIVE);
    add_type("ALPHA_SCORE", MuseDataPacketType::ALPHA_SCORE);
    add_type("BETA_SCORE", MuseDataPacketType::BETA_SCORE);
    add_type("DELTA_SCORE", MuseDataPacketType::DELTA_SCORE);
    add_type("THETA_SCORE", MuseDataPacketType::THETA_SCORE);
    add_type("GAMMA_SCORE", MuseDataPacketType::GAMMA_SCORE);
    add_type("IS_GOOD", MuseDataPacketType::IS_GOOD);
    add_type("HSI", MuseDataPacketType::HSI);
    add_type("HSI_PRECISION", MuseDataPacketType::HSI_PRECISION);
    add_type("ARTIFACTS", MuseDataPacketType::ARTIFACTS);
    data_type_combobox->SelectedIndex = 0;
}

//******************************************
#define ADDRESS "127.0.0.1"
#define PORT 7000

#define OUTPUT_BUFFER_SIZE 1024
void MainPage::sendOsc(std::string address, float value)
/* void MainPage::sendOsc(std::string address, float value) */
{
	UdpTransmitSocket transmitSocket( IpEndpointName( ADDRESS, PORT ) );

	char buffer[OUTPUT_BUFFER_SIZE];
	osc::OutboundPacketStream p( buffer, OUTPUT_BUFFER_SIZE );

	p << osc::BeginBundleImmediate
	<< osc::BeginMessage( address.c_str() )
	<< value << osc::EndMessage
	<< osc::EndBundle;

	transmitSocket.Send( p.Data(), p.Size() );
}
void MainPage::sendOscFloat(std::string tag, float value)
{
	for (auto &dire : mDirecciones) {
		UdpTransmitSocket transmitSocket(IpEndpointName(dire.ip.c_str(), dire.port));

		char buffer[OUTPUT_BUFFER_SIZE];
		osc::OutboundPacketStream p(buffer, OUTPUT_BUFFER_SIZE);

		p << osc::BeginBundleImmediate
			<< osc::BeginMessage( tag.c_str() )
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

		p << osc::BeginBundleImmediate
			<< osc::BeginMessage(tag.c_str());
			for (auto value : values) {
				p << value;
			}
			
		p << osc::EndBundle;

		transmitSocket.Send(p.Data(), p.Size());
	}
}

//Para mandar el promedio de un buffer
void MainPage::sendMeanBuffer(double *buffer , std::string tag)
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
void MainPage::enviarAlpha() {
	sendOsc("alpha", rand());
}
//////////////////////////////////



void MainPage::add_type(Platform::String^ name, MuseDataPacketType type) {
    data_type_combobox->Items->Append(name);
    name_to_type_map_.Insert(name, (int) type);
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
    }
    else {
        model_.set_version("Unknown");
    }

}

void MainPage::receive_muse_data_packet(const std::shared_ptr<MuseDataPacket> & packet, const std::shared_ptr<Muse> & muse) {
    model_.set_values(packet);
    OutputDebugString(L"MainPage::receive_muse_data_packet\n");
}

void MainPage::receive_muse_artifact_packet(const MuseArtifactPacket & packet, const std::shared_ptr<Muse> & muse) {
    model_.set_values(packet);
	//std::cout << "TEST: " << packet.blink << std::endl;
	//OutputDebugString(packet.blink);
	sendOsc("Blink", packet.blink);
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

void MainPage::data_type_selection_changed(Platform::Object^ sender,
    Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
    if (e->AddedItems->Size > 0) {

        String^ added = (String^)e->AddedItems->GetAt(0);
        MuseDataPacketType type =
            (MuseDataPacketType) name_to_type_map_.Lookup(added);

        change_data_type(type);

        switch (type) {
        case MuseDataPacketType::ACCELEROMETER:
            set_accel_ui();
            break;
        case MuseDataPacketType::GYRO:
            set_gyro_ui();
            break;
        case MuseDataPacketType::BATTERY:
            set_battery_ui();
            break;
        case MuseDataPacketType::DRL_REF:
            set_drl_ref_ui();
            break;
        case MuseDataPacketType::ARTIFACTS:
            set_artifacts_ui();
			
            break;
        default:
            // All other packet types derive from EEG data.
            set_eeg_ui();
            break;
        }
    }
    else {
        change_data_type(MuseDataPacketType::EEG);
        set_eeg_ui();
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
    /*if (model_.is_dirty()) {
        double buffer[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
        model_.get_buffer(buffer);

        line_1_data->Text = formatData(buffer[0]);
        line_2_data->Text = formatData(buffer[1]);
        line_3_data->Text = formatData(buffer[2]);
        line_4_data->Text = formatData(buffer[3]);
        line_5_data->Text = formatData(buffer[4]);
        line_6_data->Text = formatData(buffer[5]);
        connection_status->Text = Convert::to_platform_string(model_.get_connection_state());
        version->Text = Convert::to_platform_string(model_.get_version());

        model_.clear_dirty_flag();
    }*/

	//Si se ha recibido cualquier tipo de datos proceder
	if (model_.is_dirty()) {
		
		//Si ha recibido nuevos valores para Alpha relative
		if (model_.isDirtyEggAlpha()) {
			double buffer[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
			model_.getBufferEggAplha(buffer);
			double mean = getMean(buffer);
			alphaPromedio->Text = "" + mean;

			if (enviaAlpha->IsChecked) {

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
			if (enviaBeta->IsChecked) {
				sendOscFloat("/beta", mean);
			}
			model_.clearDirtyEggBeta();
		}

		//Si ha recibido nuevos valores para Delta relative
		if (model_.isDirtyEggDelta()) {
			double buffer[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
			model_.getBufferEggDelta(buffer);
			double mean = getMean(buffer);
			deltaPromedio->Text = "" + mean;
			if (enviaDelta->IsChecked) {
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
			if (enviaTheta->IsChecked) {
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
			if (enviaGamma->IsChecked) {
				sendOscFloat("/gamma", mean);
			}
			model_.clearDirtyEggGamma();
		}
		

		//Mandar los datos de artifacts
		if (model_.isDirtyBattery()) {
			double buffer[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
			model_.getBufferBattery(buffer);
			battery->Text = "" + buffer[0];
			if (Battery->IsChecked) {
				sendOscFloat("/accelerometer", (float)buffer[0]);
			}

			model_.clearDirtyAccel();
		}



		//Mandar los datos del acelerómetro
		if (model_.isDirtyAccel()) {
			double buffer[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
			model_.getBufferAccel(buffer);
			accX->Text = "" + buffer[0];
			accY->Text = "" + buffer[1];
			accZ->Text = "" + buffer[2];
			if (enviaAccelerometer->IsChecked) {
				std::vector<double> dats{ buffer[0], buffer[1], buffer[2] };
				sendOscFloatVector("", "/accelerometer", dats);
			}

			model_.clearDirtyBattery();
		}

		//Mandar los datos del gyro
		if (model_.isDirtyGyro()) {
			double buffer[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
			model_.getBufferGyro(buffer);
			gyroX->Text = "" + buffer[0];
			gyroY->Text = "" + buffer[1];
			gyroZ->Text = "" + buffer[2];
			if (enviaGyro->IsChecked) {
				std::vector<double> dats{ buffer[0], buffer[1], buffer[2] };
				sendOscFloatVector("", "/gyro", dats);
			}

			model_.clearDirtyGyro();
		}



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

void MainPage::set_accel_ui() {
    set_ui_line(line_1_label, "X", line_1_data, true);
    set_ui_line(line_2_label, "Y", line_2_data, true);
    set_ui_line(line_3_label, "Z", line_3_data, true);
    set_ui_line(line_4_label, "", line_4_data, false);
    set_ui_line(line_5_label, "", line_5_data, false);
    set_ui_line(line_6_label, "", line_6_data, false);
}

void MainPage::set_battery_ui() {
    set_ui_line(line_1_label, "CHARGE_PERCENTAGE_REMAINING", line_1_data, true);
    set_ui_line(line_2_label, "MILLIVOLTS", line_2_data, true);
    set_ui_line(line_3_label, "TEMPERATURE_CELSIUS", line_3_data, true);
    set_ui_line(line_4_label, "", line_4_data, false);
    set_ui_line(line_5_label, "", line_5_data, false);
    set_ui_line(line_6_label, "", line_6_data, false);
}

void MainPage::set_drl_ref_ui() {
    set_ui_line(line_1_label, "DRL", line_1_data, true);
    set_ui_line(line_2_label, "REF", line_2_data, true);
    set_ui_line(line_3_label, "", line_3_data, false);
    set_ui_line(line_4_label, "", line_4_data, false);
    set_ui_line(line_5_label, "", line_5_data, false);
    set_ui_line(line_6_label, "", line_6_data, false);
}

void MainPage::set_eeg_ui() {
    set_ui_line(line_1_label, "EEG1", line_1_data, true);
    set_ui_line(line_2_label, "EEG2", line_2_data, true);
    set_ui_line(line_3_label, "EEG3", line_3_data, true);
    set_ui_line(line_4_label, "EEG4", line_4_data, true);
    set_ui_line(line_5_label, "AUX_LEFT", line_5_data, true);
    set_ui_line(line_6_label, "AUX_RIGHT", line_6_data, true);
}

void MainPage::set_gyro_ui() {
    set_ui_line(line_1_label, "X", line_1_data, true);
    set_ui_line(line_2_label, "Y", line_2_data, true);
    set_ui_line(line_3_label, "Z", line_3_data, true);
    set_ui_line(line_4_label, "", line_4_data, false);
    set_ui_line(line_5_label, "", line_5_data, false);
    set_ui_line(line_6_label, "", line_6_data, false);
}

void MainPage::set_artifacts_ui() {
    set_ui_line(line_1_label, "HEADBAND_ON", line_1_data, true);
    set_ui_line(line_2_label, "BLINK", line_2_data, true);
    set_ui_line(line_3_label, "JAW_CLENCH", line_3_data, true);
    set_ui_line(line_4_label, "", line_4_data, false);
    set_ui_line(line_5_label, "", line_5_data, false);
    set_ui_line(line_6_label, "", line_6_data, false);
}

void MainPage::set_ui_line(TextBlock^ label, String^ name, TextBlock^ data, bool visible) {
    label->Text = name;
    data->Text = "0.0";

    Windows::UI::Xaml::Visibility visibility = (visible)
        ? Windows::UI::Xaml::Visibility::Visible
        : Windows::UI::Xaml::Visibility::Collapsed;
    label->Visibility = visibility;
    data->Visibility = visibility;
}

Platform::String^ MainPage::formatData(double data) const
{
    std::stringstream ss;
    ss << std::fixed << std::setprecision(DATA_PRECISION) << data;
    return Convert::to_platform_string(ss.str());
}
/*
private void toppingsCheckbox_Click(object sender, RoutedEventArgs e)
{
	string selectedToppingsText = string.Empty;
	CheckBox[] checkboxes = new CheckBox[]{ pepperoniCheckbox, beefCheckbox,
		mushroomsCheckbox, onionsCheckbox };
	foreach(CheckBox c in checkboxes)
	{
		if (c.IsChecked == true)
		{
			if (selectedToppingsText.Length > 1)
			{
				selectedToppingsText += ", ";
			}
			selectedToppingsText += c.Content;
		}
	}
	toppingsList.Text = selectedToppingsText;
}
*/

void GettingData::MainPage::Button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (listaDirecciones->Items->Size < 1) {
		return;
	}

	if (mDirecciones.size() != listaDirecciones->Items->Size) {
		return;
	}

	mDirecciones.erase(mDirecciones.begin() + listaDirecciones->SelectedIndex);
	listaDirecciones->Items->RemoveAt(listaDirecciones->SelectedIndex);
}



void GettingData::MainPage::addIP_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (nuevaIp->Text->ToString() == "" ||  nuevoPuerto->Text->ToString() == "")
		return;

	Platform::String^ compuesta = nuevaIp->Text->ToString() + ":" + nuevoPuerto->Text->ToString();

	listaDirecciones->Items->Append(compuesta);

	Platform::String^ fooRT = nuevaIp->Text->ToString();
	std::wstring fooW(fooRT->Begin());
	std::string fooA(fooW.begin(), fooW.end());

	direccion dire;
	dire.ip = fooA;
	dire.port = _wtoi(nuevoPuerto->Text->ToString()->Data());
	mDirecciones.push_back(dire);

}
