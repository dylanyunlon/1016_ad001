#pragma once
#include "uart/ProtocolSender.h"
/*
*此文件由GUI工具生成
*文件功能：用于处理用户的逻辑相应代码
*功能说明：
*========================onButtonClick_XXXX
当页面中的按键按下后系统会调用对应的函数，XXX代表GUI工具里面的[ID值]名称，
如Button1,当返回值为false的时候系统将不再处理这个按键，返回true的时候系统将会继续处理此按键。比如SYS_BACK.
*========================onSlideWindowItemClick_XXXX(int index)
当页面中存在滑动窗口并且用户点击了滑动窗口的图标后系统会调用此函数,XXX代表GUI工具里面的[ID值]名称，
如slideWindow1;index 代表按下图标的偏移值
*========================onSeekBarChange_XXXX(int progress)
当页面中存在滑动条并且用户改变了进度后系统会调用此函数,XXX代表GUI工具里面的[ID值]名称，
如SeekBar1;progress 代表当前的进度值
*========================ogetListItemCount_XXXX()
当页面中存在滑动列表的时候，更新的时候系统会调用此接口获取列表的总数目,XXX代表GUI工具里面的[ID值]名称，
如List1;返回值为当前列表的总条数
*========================oobtainListItemData_XXXX(ZKListView::ZKListItem *pListItem, int index)
 当页面中存在滑动列表的时候，更新的时候系统会调用此接口获取列表当前条目下的内容信息,XXX代表GUI工具里面的[ID值]名称，
如List1;pListItem 是贴图中的单条目对象，index是列表总目的偏移量。具体见函数说明
*========================常用接口===============
*LOGD(...)  打印调试信息的接口
*mTextXXXPtr->setText("****") 在控件TextXXX上显示文字****
*mButton1Ptr->setSelected(true); 将控件mButton1设置为选中模式，图片会切换成选中图片，按钮文字会切换为选中后的颜色
*mSeekBarPtr->setProgress(12) 在控件mSeekBar上将进度调整到12
*mListView1Ptr->refreshListView() 让mListView1 重新刷新，当列表数据变化后调用
*mDashbroadView1Ptr->setTargetAngle(120) 在控件mDashbroadView1上指针显示角度调整到120度
*
* 在Eclipse编辑器中  使用 “alt + /”  快捷键可以打开智能提示
*/

#include "net/context.h"
#include "link/context.h"
#include "uart/context.h"
#include "bt/context.h"
#include "media/audio_context.h"
#include "media/media_context.h"
#include "media/music_player.h"
#include "media/media_parser.h"
#include "system/setting.h"
#include "system/fm_emit.h"
#include "system/reverse.h"
#include "manager/LanguageManager.h"
#include "manager/ConfigManager.h"
#include "storage/StoragePreferences.h"
#include "misc/storage.h"
#include "fy/files.hpp"
#include "net/NetManager.h"
#include "os/MountMonitor.h"
#include "system/usb_monitor.h"
#include "tire/tire_parse.h"
#include "sysapp_context.h"
#include "utils/BitmapHelper.h"
#include <base/ui_handler.h>
#include <base/mount_notification.h>
#include "system/hardware.h"

#define WIFIMANAGER			NETMANAGER->getWifiManager()

#define QUERY_LINK_AUTH_TIMER	3
#define SWITCH_ADB_TIMER	4
#define MUSIC_ERROR_TIMER	20

static bt_cb_t _s_bt_cb;
static bool _s_need_reopen_linkview;

static void _register_timer_fun(int id, int time) {
	mActivityPtr->registerUserTimer(id, time); // @suppress("无效参数")
}

static void _unregister_timer_fun(int id) {
	mActivityPtr->unregisterUserTimer(id); // @suppress("无效参数")
}

static void entry_lylink_ftu() {
	if (!sys::reverse_does_enter_status()) {
		EASYUICONTEXT->openActivity("lylinkviewActivity");
		_s_need_reopen_linkview = false;
	} else {
		LOGD("Is reverse status !!!\n");
		lk::video_stop();
		_s_need_reopen_linkview = true;
	}
}

static void _lylink_callback(LYLINKAPI_EVENT evt, int para0, void *para1) {
	switch (evt) {
	case LYLINK_LINK_ESTABLISH:
		LOGD("LYLINK_LINK_ESTABLISH %s", lk::_link_type_to_str((LYLINK_TYPE_E) para0));
		EASYUICONTEXT->hideStatusBar();		//隐藏蓝牙来电界面
		if (LINK_TYPE_AIRPLAY == para0 || LINK_TYPE_MIRACAST == para0 || LINK_TYPE_WIFILY == para0 || LINK_TYPE_WIFICP == para0) {
			if (bt::is_on()) {
				bt::power_off();
			}
			entry_lylink_ftu();
		}
		break;
	case LYLINK_LINK_DISCONN:
		LOGD("LYLINK_LINK_DISCONN........... %s", lk::_link_type_to_str((LYLINK_TYPE_E) para0));
		if (LINK_TYPE_AIRPLAY == para0 || LINK_TYPE_MIRACAST == para0 || LINK_TYPE_WIFILY == para0 || LINK_TYPE_WIFICP == para0) {
			if (!bt::is_on()) {
				bt::power_on();
			}
		}
		bt::query_state();
		EASYUICONTEXT->closeActivity("lylinkviewActivity");
		break;
	case LYLINK_PHONE_CONNECT:
		LOGD("LYLINK_PHONE_CONNECT %s", lk::_link_type_to_str((LYLINK_TYPE_E) para0));
		if (para0 == LINK_TYPE_WIFIAUTO || para0 == LINK_TYPE_WIFICP) {
			LOGD("You should open AP now.");
		}
		break;
	case LYLINK_FOREGROUND:
		LOGD("LYLINK_FOREGROUND");
		entry_lylink_ftu();
		break;
	case LYLINK_BACKGROUND:
	case LYLINK_HID_COMMAND:{
		if (evt == LYLINK_BACKGROUND) {
			LOGD("[main] LYLINK_BACKGROUND\n");
		} else {
			LOGD("[main] LYLINK_HID_COMMAND");
		}

		const char *app = EASYUICONTEXT->currentAppName();
		if (app && (strcmp(app, "lylinkviewActivity") == 0)) {
			EASYUICONTEXT->goHome();
		} else {
			EASYUICONTEXT->closeActivity("lylinkviewActivity");
		}
		_s_need_reopen_linkview = false;
	}
		break;
	case LYLINK_PHONE_DISCONN: 				// 蓝牙断开,不用处理
		LOGD("LYLINK_PHONE_DISCONN............. %s", lk::_link_type_to_str((LYLINK_TYPE_E) para0));
		lylinkapi_gocsdk("IA\r\n", strlen("IA\r\n"));
		break;
	default:
		break;
	}
}

static void _reverse_status_cb(int status) {
	LOGD("reverse status %d\n", status);
	base::runInUiThreadUniqueDelayed("rear_view_detection", [](){
		int status = sys::reverse_does_enter_status();
		LOGD("[main] reverse_status %d\n", status);
		if (status == REVERSE_STATUS_ENTER) {
			EASYUICONTEXT->openActivity("reverseActivity");
		} else {
			EASYUICONTEXT->closeActivity("reverseActivity");
			if (_s_need_reopen_linkview) {
				_s_need_reopen_linkview = false;
				if (lk::is_connected()) {
					EASYUICONTEXT->openActivity("lylinkviewActivity");
				}
			}
		}

	}, 500);
}

static void parser() {
	std::string cur_play_file = media::music_get_current_play_file();
	id3_info_t info;
	memset(&info, 0, sizeof(id3_info_t));
	bool isTrue = media::parse_id3_info(cur_play_file.c_str(), &info);
	if (isTrue && strcmp(info.title, "") != 0) {
		mtitleTextViewPtr->setText(info.title);
	} else {
		mtitleTextViewPtr->setText(fy::files::get_file_name(cur_play_file));
	}
	if (isTrue) {
		(strcmp(info.artist, "") == 0) ? martistTextViewPtr->setTextTr("Unknown") : martistTextViewPtr->setText(info.artist);
	} else {
		martistTextViewPtr->setTextTr("Unknown");
	}
	isTrue = media::parse_id3_pic(cur_play_file.c_str(), "/tmp/music.jpg");
	mmusicPicTextViewPtr->setBackgroundPic(NULL);
	mmusicPicTextViewPtr->setBackgroundPic(isTrue ? "/tmp/music.jpg" : CONFIGMANAGER->getResFilePath("/HomePage/icon_media_cover_n.png").c_str());
	martistTextViewPtr->setLongMode(ZKTextView::E_LONG_MODE_SCROLL_CIRCULAR);
}

static void _update_music_info() {
	bt_music_t music_info = bt::get_music_info();
	mtitleTextViewPtr->setText(music_info.title);
	martistTextViewPtr->setText(music_info.artist);
	martistTextViewPtr->setLongMode(ZKTextView::E_LONG_MODE_SCROLL_CIRCULAR);
}

static void _update_music_progress() {
	bt_music_t music_info = bt::get_music_info();

	mPlayProgressSeekbarPtr->setMax(music_info.duration);
	mPlayProgressSeekbarPtr->setProgress(music_info.curpos);
}

static void _bt_music_cb(bt_music_state_e state) {
	if (bt::music_is_playing()) {
		_update_music_info();
		_update_music_progress();
		mtitleTextViewPtr->setLongMode(ZKTextView::E_LONG_MODE_SCROLL_CIRCULAR);
		mtitleTextViewPtr->setTextColor(0xFF00FF40);
		mButtonPlayPtr->setSelected(true);
		sys::setting::set_music_play_dev(E_AUDIO_TYPE_BT_MUSIC);
	} else {
		mtitleTextViewPtr->setLongMode(ZKTextView::E_LONG_MODE_SCROLL_CIRCULAR);
		mtitleTextViewPtr->setTextColor(0xFFFFFFFF);
		mButtonPlayPtr->setSelected(false);
	}
}

static void _music_play_status_cb(music_play_status_e status) {
	switch (status) {
	case E_MUSIC_PLAY_STATUS_STARTED:     	// 播放开始
		parser();
		sys::setting::set_music_play_dev(E_AUDIO_TYPE_MUSIC);
		mButtonPlayPtr->setSelected(true);
		mtitleTextViewPtr->setLongMode(ZKTextView::E_LONG_MODE_SCROLL_CIRCULAR);
		mtitleTextViewPtr->setTextColor(0xFF00FF00);
		mPlayProgressSeekbarPtr->setMax(media::music_get_duration() / 1000);
		mPlayProgressSeekbarPtr->setProgress(0);
		break;
	case E_MUSIC_PLAY_STATUS_RESUME:     	// 恢复播放
		parser();
		sys::setting::set_music_play_dev(E_AUDIO_TYPE_MUSIC);
		mPlayProgressSeekbarPtr->setMax(media::music_get_duration() / 1000);
		mButtonPlayPtr->setSelected(true);
		mtitleTextViewPtr->setLongMode(ZKTextView::E_LONG_MODE_SCROLL_CIRCULAR);
		mtitleTextViewPtr->setTextColor(0xFF00FF00);
		break;
	case E_MUSIC_PLAY_STATUS_STOP:       	// 停止播放
		mPlayProgressSeekbarPtr->setMax(media::music_get_duration() / 1000);
		mButtonPlayPtr->setSelected(false);
		mtitleTextViewPtr->setLongMode(ZKTextView::E_LONG_MODE_SCROLL_CIRCULAR);
		mtitleTextViewPtr->setTextColor(0xFFFFFFFF);
		mtitleTextViewPtr->setTextTr("Unknown");
		martistTextViewPtr->setTextTr("Unknown");
		mmusicPicTextViewPtr->setBackgroundPic("/HomePage/icon_media_cover_n.png");
		break;
	case E_MUSIC_PLAY_STATUS_PAUSE:      	// 暂停播放
		mtitleTextViewPtr->setLongMode(ZKTextView::E_LONG_MODE_NONE);
		mButtonPlayPtr->setSelected(false);
		mtitleTextViewPtr->setTextColor(0xFFFFFFFF);
		break;
	case E_MUSIC_PLAY_STATUS_COMPLETED:  	// 播放结束
		LOGE("[main] music play completed, will play next\n");
		media::music_next();
		break;
	case E_MUSIC_PLAY_STATUS_ERROR:      	// 播放错误
//		LOGE("music play error, will play next\n");
//		mActivityPtr->registerUserTimer(MUSIC_ERROR_TIMER, 3000);
		break;
	}
}

static void _bt_call_cb(bt_call_state_e state) {
	if (state != E_BT_CALL_STATE_IDLE) {
//		if (EASYUICONTEXT->isScreensaverEnable()) {
//			EASYUICONTEXT->screensaverOff();
//		}
		if (lk::get_lylink_type() == LINK_TYPE_WIFIAUTO) {
			const char *app = EASYUICONTEXT->currentAppName();
			if (!app) return;
			if(strcmp(app, "reverseActivity") == 0) {
				_s_need_reopen_linkview = true;
			} else if(strcmp(app, "lylinkviewActivity") != 0) {
				EASYUICONTEXT->openActivity("lylinkviewActivity");
			}
		}
	}
}

static void _bt_add_cb() {
	_s_bt_cb.call_cb = _bt_call_cb;
	_s_bt_cb.music_cb = _bt_music_cb;
	bt::add_cb(&_s_bt_cb);
}

static void _bt_remove_cb() {
	bt::remove_cb(&_s_bt_cb);
}

static bool _show_sys_info(unsigned long *freeram) {
	struct sysinfo info;
	int ret = 0;
	ret = sysinfo(&info);
	if(ret != 0) {
		return false;
	}
	*freeram = info.freeram;
	return true;
}

static void ctrl_UI_init() {
	EASYUICONTEXT->hideStatusBar();
}

namespace{
class AppSlidePageChangeListener : public ZKSlideWindow::ISlidePageChangeListener {
protected:
	virtual void onSlidePageChange(ZKSlideWindow *pSlideWindow, int page) {
		mStatusRadioGroupPtr->setCheckedID((page == 0) ? ID_MAIN_RadioButton0 : ID_MAIN_RadioButton1);
	}
};

void onExtsdMounted(int status,
    const std::string& mount_point) {
  switch (status) {
    case MountMonitor::E_MOUNT_STATUS_MOUNTED:
    	if (fy::files::exists(
    			fy::path::join(TFCARD_MOUNT_POINT, PRODUCT_TEST_FILE_NAME).c_str())) {
//    		DELAY(3000);
    		EASYUICONTEXT->openActivity("TestActivity");
    	}
      break;
    case MountMonitor::E_MOUNT_STATUS_REMOVE:
      break;
    case MountMonitor::E_MOUNT_STATUS_CHECKING:
      break;
    default:
    	   break;
  }
}
}
static AppSlidePageChangeListener _s_app_slide_page_change_listener;

static void set_back_pic() {
	bitmap_t *bg_bmp = NULL;
	BitmapHelper::loadBitmapFromFile(bg_bmp, CONFIGMANAGER->getResFilePath("/HomePage/car_home_wallpaper_first.jpg").c_str(), 3);
	mTextViewBgPtr->setBackgroundBmp(bg_bmp);
}

static void _preload_resources() {
	const char *pic_tab[] = {
		"/res/font/sans.ttf",
		"navi/bg_bt_n.png",
		"navi/bg_bt_p.png",
		"navi/bg_eq_n.png",
		"navi/bg_eq_p.png",
		"navi/bg_fm_n.png",
		"navi/bg_fm_p.png",
		"navi/bg_screen_off_n.png",
		"navi/bg_screen_off_p.png",
		"navi/icon_btvoice.png",
		"navi/icon_light.png",
		"navi/icon_setting_n.png",
		"navi/icon_setting_p.png",
		"navi/icon_voice.png",
		"navi/progress_n.png",
		"navi/progress_p.png",
	};

	LOGD("[main] preload resources start\n");

	size_t size = TAB_SIZE(pic_tab);
	for (size_t i = 0; i < size; ++i) {
		if (i == 0) {
			fy::cache_file(pic_tab[i]);
		} else {
			fy::cache_file(CONFIGMANAGER->getResFilePath(pic_tab[i]));
		}
	}

	LOGD("[main] preload resources end\n");

	return ;
}
/**
 * 注册定时器
 * 填充数组用于注册定时器
 * 注意：id不能重复
 */
static S_ACTIVITY_TIMEER REGISTER_ACTIVITY_TIMER_TAB[] = {
//	{0,  1000}, //定时器id=0, 时间间隔1秒
	{1,  1000},
	{QUERY_LINK_AUTH_TIMER, 6000},
	{SWITCH_ADB_TIMER, 1000}, // 延迟打开ADB
};

/**
 * 当界面构造时触发
 */
static void onUI_init() {
    //Tips :添加 UI初始化的显示代码到这里,如:mText1Ptr->setText("123");
	// f133-B 文件系统加载慢，导致上电第一次滑动ui很慢，提前cach文件
//	system("cp /res/font/sans.ttf /dev/null");
	_preload_resources();
	ctrl_UI_init();

	// 系统设置初始化
	sys::setting::init();
	// 打开温控线程
	sys::hw::init();

	// 串口初始化
	uart::init();
	// 上电mute
	uart::set_amplifier_mute(0);

	// 蓝牙初始化
	bt::init();

	// 网络初始化
	net::init();

	// 媒体初始化
	media::init();
	media::music_add_play_status_cb(_music_play_status_cb);

	// 启动手机互联
	lk::add_lylink_callback(_lylink_callback);
	lk::start_lylink();

	app::attach_timer(_register_timer_fun, _unregister_timer_fun);

	// 启动倒车检测
	sys::reverse_add_status_cb(_reverse_status_cb);
	sys::reverse_detect_start();

	_bt_add_cb();
	bt::query_state();

	media::music_add_play_status_cb(_music_play_status_cb);
	mTextView1Ptr->setTouchPass(true);
	martistTextViewPtr->setTouchPass(true);
	mappSlideWindowPtr->setSlidePageChangeListener(&_s_app_slide_page_change_listener);
	mStatusRadioGroupPtr->setCheckedID(ID_MAIN_RadioButton0);

	if(bt::is_calling()){
		bt::call_vol(audio::get_lylink_call_vol());
	}

	base::UiHandler::implementTimerRegistration([]() {
		mActivityPtr->registerUserTimer(base::TIMER_UI_HANDLER, 0); // @suppress("无效参数")
	});

	base::runInUiThreadDelayed("delayInit", [](){
		static base::MountNotification mount_extsd(TFCARD_MOUNT_POINT,
				onExtsdMounted,
				MountMonitor::E_MOUNT_STATUS_MOUNTED,
				MountMonitor::E_MOUNT_STATUS_REMOVE,
				MountMonitor::E_MOUNT_STATUS_CHECKING);
	}, 3000);
}

/**
 * 当切换到该界面时触发
 */
static void onUI_intent(const Intent *intentPtr) {
    if (intentPtr != NULL) {
        //TODO
    }
}

/*
 * 当界面显示时触发
 */
static void onUI_show() {
	int curPos = -1;
//	mDigitalClock2Ptr->setVisible(true);
    set_back_pic();
	if (sys::setting::get_music_play_dev() == E_AUDIO_TYPE_BT_MUSIC) {
		_update_music_info();
		_update_music_progress();
		if (bt::music_is_playing()) {
			mButtonPlayPtr->setSelected(true);
		}
	} else if (sys::setting::get_music_play_dev() == E_AUDIO_TYPE_MUSIC) {
		parser();
		if (media::music_is_playing()) {
			mButtonPlayPtr->setSelected(true);
			curPos = media::music_get_current_position() / 1000;
			mPlayProgressSeekbarPtr->setMax(media::music_get_duration() / 1000);
			mtitleTextViewPtr->setLongMode(ZKTextView::E_LONG_MODE_SCROLL_CIRCULAR);
			mtitleTextViewPtr->setTextColor(0xFF00FF00);
		}
	}
    if (curPos >= 0) {
    	mPlayProgressSeekbarPtr->setProgress(curPos);
    }
	if (!app::is_show_topbar()) {
		sys::setting::set_reverse_topbar_show(true);
		app::show_topbar();
	}

}

/*
 * 当界面隐藏时触发
 */
static void onUI_hide() {
	mTextViewBgPtr->setBackgroundBmp(NULL);
}

/*
 * 当界面完全退出时触发
 */
static void onUI_quit() {
	lk::remove_lylink_callback(_lylink_callback);
	media::music_remove_play_status_cb(_music_play_status_cb);
	mPlayProgressSeekbarPtr->setSeekBarChangeListener(NULL);
	mappSlideWindowPtr->setSlidePageChangeListener(NULL);
	mTextViewBgPtr->setBackgroundBmp(NULL);
	_bt_remove_cb();
}

/**
 * 串口数据回调接口
 */
static void onProtocolDataUpdate(const SProtocolData &data) {

}

/**
 * 定时器触发函数
 * 不建议在此函数中写耗时操作，否则将影响UI刷新
 * 参数： id
 *         当前所触发定时器的id，与注册时的id相同
 * 返回值: true
 *             继续运行当前定时器
 *         false
 *             停止运行当前定时器
 */
static bool onUI_Timer(int id) {
	if (app::on_timer(id)) {
		return false;
	}
	switch (id) {
	case 0: {
		unsigned long freeram = 0;
		bool ret = _show_sys_info(&freeram);
		if(ret) {
			LOGD("-----------Current MemFree: %ldKB---------------", freeram >> 10);
		} else {
			LOGD("-----------get MemFree info fail----------------");
		}
	}
		break;
	case 1: {
        int curPos = -1;
        if (sys::setting::get_music_play_dev() == E_AUDIO_TYPE_MUSIC) {
            if (media::music_is_playing()) {
                curPos = media::music_get_current_position() / 1000;
            }
            if (curPos >= 0) {
            	mPlayProgressSeekbarPtr->setProgress(curPos);
            }
        }
	}
		break;
	case QUERY_LINK_AUTH_TIMER:		// 查询互联授权状态
		lk::query_is_authorized();
		return false;
	case SWITCH_ADB_TIMER: {
		if (strcmp("UpgradeActivity", EASYUICONTEXT->currentAppName()) == 0) {
			app::hide_topbar();
		}
		if (sys::setting::is_usb_adb_enabled()) {
			if (sys::get_usb_mode() != E_USB_MODE_DEVICE) {
				sys::change_usb_mode(E_USB_MODE_DEVICE);
			}
		} else {
			sys::set_usb_config(E_USB_MODE_HOST);
		}
		sys::change_usb_mode(E_USB_MODE_DEVICE);

		// unmute
		uart::set_amplifier_mute(1);
	}
		return false;
	case MUSIC_ERROR_TIMER:
		media::music_next(true);
		return false;
    case base::TIMER_UI_HANDLER:
      return base::UiHandler::onTimer();
      break;
	default:
		break;
	}
    return true;
}

/**
 * 有新的触摸事件时触发
 * 参数：ev
 *         新的触摸事件
 * 返回值：true
 *            表示该触摸事件在此被拦截，系统不再将此触摸事件传递到控件上
 *         false
 *            触摸事件将继续传递到控件上
 */
static bool onmainActivityTouchEvent(const MotionEvent &ev) {
	LayoutPosition pos = EASYUICONTEXT->getNaviBar()->getPosition();
	if (pos.mTop != -pos.mHeight) {	return false; } 	// 导航栏下滑了
	switch (ev.mActionStatus) {
	case MotionEvent::E_ACTION_DOWN:	//触摸按下
		break;
	case MotionEvent::E_ACTION_MOVE:	//触摸滑动
		break;
	case MotionEvent::E_ACTION_UP:  	//触摸抬起
		break;
	default:
		break;
	}
	return false;
}
static bool onButtonClick_NextButton(ZKButton *pButton) {
    LOGD(" ButtonClick NextButton !!!\n");
	if (lk::is_connected()) {
		mlinkTipsWindowPtr->showWnd();
		return false;
	}
	if (sys::setting::get_music_play_dev() == E_AUDIO_TYPE_MUSIC) {
	    media::music_next(true);
	} else if (sys::setting::get_music_play_dev() == E_AUDIO_TYPE_BT_MUSIC) {
		bt::music_next();
	}
    return false;
}

static bool onButtonClick_ButtonPlay(ZKButton *pButton) {
    LOGD(" ButtonClick ButtonPlay !!!\n");

	if (lk::is_connected()) {
		mlinkTipsWindowPtr->showWnd();
		return false;
	}

	if (sys::setting::get_music_play_dev() == E_AUDIO_TYPE_MUSIC) {
	    if (media::music_get_play_index() == -1) {
	    	return false;
	    } else if (media::music_is_playing()) {
	        media::music_pause();
	    } else {
	    	media::music_resume();
	    }
	} else if (sys::setting::get_music_play_dev() == E_AUDIO_TYPE_BT_MUSIC) {
	    bt::music_is_playing() ? bt::music_pause() : bt::music_play();
	}
    return false;
}

static bool onButtonClick_PrevButton(ZKButton *pButton) {
    LOGD(" ButtonClick PrevButton !!!\n");
	if (lk::is_connected()) {
		mlinkTipsWindowPtr->showWnd();
		return false;
	}
	if (sys::setting::get_music_play_dev() == E_AUDIO_TYPE_MUSIC) {
	    media::music_prev(true);
	} else if (sys::setting::get_music_play_dev() == E_AUDIO_TYPE_BT_MUSIC) {
		bt::music_prev();
	}
    return false;
}

static void onProgressChanged_PlayProgressSeekbar(ZKSeekBar *pSeekBar, int progress) {
    //LOGD(" ProgressChanged PlayProgressSeekbar %d !!!\n", progress);
}

static bool onButtonClick_Setting(ZKButton *pButton) {
    LOGD(" ButtonClick Setting !!!\n");
    EASYUICONTEXT->openActivity("settingsActivity");
    return false;
}

static bool onButtonClick_ToMusic(ZKButton *pButton) {
    LOGD(" ButtonClick ToMusic !!!\n");
	if (lk::is_connected()) {
		mlinkTipsWindowPtr->showWnd();
		return false;
	}

	if (sys::setting::get_music_play_dev() == E_AUDIO_TYPE_MUSIC) {
	    EASYUICONTEXT->openActivity("musicActivity");
	} else if (sys::setting::get_music_play_dev() == E_AUDIO_TYPE_BT_MUSIC) {
	    EASYUICONTEXT->openActivity("btMusicActivity");
	}
    return false;
}

static void open_linkhelp_activity(link_mode_e mode) {
	Intent *i = new Intent;
	i->putExtra("link_mode", fy::format("%d", mode));
	LOGD("[main] choose link mode %s\n", sys::setting::get_link_mode_str(mode));
	EASYUICONTEXT->openActivity("linkhelpActivity", i);
}

static void _change_link_app(link_mode_e mode) {
	switch (mode) {
	case E_LINK_MODE_HICAR:
	case E_LINK_MODE_ANDROIDAUTO:
	case E_LINK_MODE_CARPLAY:
		if (net::get_mode() != E_NET_MODE_AP) { net::change_mode(E_NET_MODE_AP);}
		break;
	case E_LINK_MODE_AIRPLAY:
		if (net::get_mode() != E_NET_MODE_AP) { net::change_mode(E_NET_MODE_AP); }
		break;
	case E_LINK_MODE_CARLIFE:
		if (net::get_mode() != E_NET_MODE_WIFI) { net::change_mode(E_NET_MODE_WIFI); }
		break;
	case E_LINK_MODE_MIRACAST:
	case E_LINK_MODE_LYLINK:
		if (net::get_mode() != E_NET_MODE_P2P) { net::change_mode(E_NET_MODE_P2P); }
		break;
	default:
		break;
	}
	open_linkhelp_activity(mode);
}

static void open_link_activity(link_mode_e mode) {
	LYLINK_TYPE_E link_type = lk::get_lylink_type();
	switch(mode) {
	case E_LINK_MODE_CARPLAY:
		if ((link_type == LINK_TYPE_WIFICP) || (link_type == LINK_TYPE_USBCP)) {
			EASYUICONTEXT->openActivity("lylinkviewActivity");
			return;
		}
		break;
	case E_LINK_MODE_ANDROIDAUTO:
		if ((link_type == LINK_TYPE_WIFIAUTO) || (link_type == LINK_TYPE_USBAUTO)) {
			EASYUICONTEXT->openActivity("lylinkviewActivity");
			return;
		}
		break;
	case E_LINK_MODE_CARLIFE:
		if ((link_type == LINK_TYPE_WIFILIFE) || (link_type == LINK_TYPE_USBLIFE)) {
			EASYUICONTEXT->openActivity("lylinkviewActivity");
			return;
		}
		break;
	case E_LINK_MODE_HICAR:
		if ((link_type == LINK_TYPE_WIFIHICAR) || (link_type == LINK_TYPE_USBHICAR)) {
			EASYUICONTEXT->openActivity("lylinkviewActivity");
			return;
		}
		break;
	case E_LINK_MODE_MIRACAST:
		if (link_type == LINK_TYPE_MIRACAST) {
			EASYUICONTEXT->openActivity("lylinkviewActivity");
			return;
		}
		break;
	case E_LINK_MODE_LYLINK:
		if (link_type == LINK_TYPE_WIFILY) {
			EASYUICONTEXT->openActivity("lylinkviewActivity");
			return;
		}
		break;
	case E_LINK_MODE_AIRPLAY:
		if (link_type == LINK_TYPE_AIRPLAY) {
			EASYUICONTEXT->openActivity("lylinkviewActivity");
			return;
		}
		break;
	default:
		break;
	}
	if (lk::is_connected()) {
		mlinkTipsWindowPtr->showWnd();
		return;
	}

	if (mode == E_LINK_MODE_AIRPLAY || mode == E_LINK_MODE_LYLINK || mode ==E_LINK_MODE_MIRACAST) {
		if (bt::is_on()) {
			bt::power_off();
		}
	} else {
		if (!bt::is_on()) {
			bt::power_on();
		}
	}

	open_linkhelp_activity(mode);
//	_change_link_app(mode);
}

static bool onButtonClick_Button1(ZKButton *pButton) {
    LOGD(" ButtonClick Button1 !!!\n");
    EASYUICONTEXT->openActivity("mcubtUpdActivity");
//    EASYUICONTEXT->openActivity("TestRecordActivity");
    return false;
}

static bool onButtonClick_Button2(ZKButton *pButton) {
    LOGD(" ButtonClick Button2 !!!\n");
   // bt::query_mf();
    EASYUICONTEXT->openActivity("soundEffectActivity");
    return false;
}

static void onCheckedChanged_StatusRadioGroup(ZKRadioGroup* pRadioGroup, int checkedID) {
    LOGD(" RadioGroup StatusRadioGroup checked %d", checkedID);
}
static void onSlideItemClick_appSlideWindow(ZKSlideWindow *pSlideWindow, int index) {
    //LOGD(" onSlideItemClick_ appSlideWindow %d !!!\n", index);
	switch(index) {
	case 0:
	    LOGD(" ButtonClick CPButton !!!\n");
	    open_link_activity(E_LINK_MODE_CARPLAY);
		break;
	case 1:
		LOGD(" ButtonClick AAButton !!!\n");
		open_link_activity(E_LINK_MODE_ANDROIDAUTO);
		break;
	case 2:
	    LOGD(" ButtonClick APButton !!!\n");
	    open_link_activity(E_LINK_MODE_AIRPLAY);
    	break;
	case 3:
	    LOGD(" ButtonClick ACButton !!!\n");
	    open_link_activity(E_LINK_MODE_LYLINK);
 		break;
	case 4:
	    LOGD(" ButtonClick MCButton !!!\n");
	    open_link_activity(E_LINK_MODE_MIRACAST);
		break;
	case 5:
	    LOGD(" ButtonClick BtMusicButton !!!\n");
		if (lk::is_connected()) {
			mlinkTipsWindowPtr->showWnd();
			break;
		}
	    EASYUICONTEXT->openActivity("btsettingActivity");
 		break;
	case 6:  {
	    LOGD(" ButtonClick MusicButton !!!\n");
		if (lk::is_connected()) {
			mlinkTipsWindowPtr->showWnd();
			break;
		}
	    EASYUICONTEXT->openActivity("musicActivity");
	}
		break;
	case 7:{
	    LOGD(" ButtonClick VideoButton !!!\n");
		if (lk::is_connected()) {
			mlinkTipsWindowPtr->showWnd();
			break;
		}
	    EASYUICONTEXT->openActivity("videoActivity");
	}
		break;
	case 8: {
		LOGD(" ButtonClick PhotoAlbum !!!\n");
		if (lk::is_connected()) {
			mlinkTipsWindowPtr->showWnd();
			break;
		}
		EASYUICONTEXT->openActivity("PhotoAlbumActivity");
	}
		break;
	case 9:
	    LOGD(" ButtonClick setting !!!\n");
	    EASYUICONTEXT->openActivity("setshowActivity");
		break;
	case 10:
	    LOGD(" ButtonClick FM !!!\n");
	    EASYUICONTEXT->openActivity("FMemitActivity");
		break;
//	case 11:
//		LOGD(" ButtonClick Tire !!!\n");
//		EASYUICONTEXT->openActivity("tirePressureActivity");
//		break;
//	case 12:
//	    LOGD(" ButtonClick CLButton !!!\n");
//	    open_link_activity(E_LINK_MODE_CARLIFE);
//		break;
	default: break;
	}
}
static void onProgressChanged_PlayVolSeekBar(ZKSeekBar *pSeekBar, int progress) {
    //LOGD(" ProgressChanged PlayVolSeekBar %d !!!\n", progress);
}
