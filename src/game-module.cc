#include "game-module.h"

#include <cstddef>
#include <chrono>
#include <thread>

#include "ui-handler.h"
#include "user-data.h"
#include "timer-handler.h"
#include "service-manager.h"
#include "object-defined.h"

extern "C" {
#include <ncurses.h>
#include <unistd.h>  // for development
}

namespace cli_tetris {
/* GameState Class ===================================================================================== */

GameState::GameState(GameManager& supervisor, UserData& user_player, UiHandler& ui, TimerHandler& timer)
    : supervisor_(supervisor), player_(user_player), ui_(ui), timer_(timer) {}

GameState::~GameState() {}

/* GameState - StartState Class ===================================================================================== */

StartState::StartState(GameManager& supervisor, UserData& user_player, UiHandler& ui, TimerHandler& timer)
    : GameState(supervisor, user_player, ui, timer) {
}

void StartState::MoveStateHandler(StateCode where) {
    this->FinishProcess();
    supervisor_.ChangeSelcet(where);
}

void StartState::Initialize() {
    //대기시간 설정
    ready_milliseconds_ = 5000;

    // Get Timer accessor
    accessor_list_.push_back(timer_.CreateTimer());
}

// ProcessResult StartState::InputProcess() {
//     //아무것도 입력받지 않습니다.
//     return ProcessResult::kNothing;
// }
void StartState::EnterProcess() {
    ui_.ClearScreen();

    // Timer 10초 설정.
    timer_.SetTimer(accessor_list_.at(0), 1, 0);

    // Drawing할 Ui object 등록
    ui_object_list_.push_back(std::make_unique<StandbyUI>(ui_.getCurrentScreenSize()));

    // 최초에 한번 Draw 합니다.
    this->RenderProcess();
}
// ProcessResult StartState::UpdateProcess(std::chrono::duration<int64_t, std::nano> diff) {
ProcessResult StartState::UpdateProcess() {
    // timer 설정값 현재 10초 만큼 대기 후, MenuState로 이동.
    if (accessor_list_.at(0).IsAlive() && !accessor_list_.at(0).IsRunning()) {
        MoveStateHandler(StateCode::kMenu);
        return ProcessResult::kChangeState;
    }

    return ProcessResult::kNothing;
}

void StartState::RenderProcess() {
    for (auto itr = ui_object_list_.begin(); itr != ui_object_list_.end(); ++itr) {
        if (!(*itr)->IsChanged()) continue;

        ui_.Draw((*itr).get());
    }
}

void StartState::FinishProcess() {
    // 할당한 Object를 모두 해제합니다.
    ui_object_list_.erase(ui_object_list_.begin(), ui_object_list_.end());
}

/* GameState - EndState Class ===================================================================================== */

EndState::EndState(GameManager& supervisor, UserData& user_player, UiHandler& ui, TimerHandler& timer)
    : GameState(supervisor, user_player, ui, timer) {}

void EndState::MoveStateHandler(StateCode where) {
    this->FinishProcess();
    supervisor_.ChangeSelcet(where);
}

void EndState::Initialize() {
    //대기시간 설정
    ready_milliseconds_ = 10000;

    // Get Timer accessor
    accessor_list_.push_back(timer_.CreateTimer());
}

// ProcessResult EndState::InputProcess() {
//     //아무것도 입력받지 않습니다.
//     return ProcessResult::kNothing;
// }
void EndState::EnterProcess() {
    ui_.ClearScreen();

    // Timer 5초 설정.
    timer_.SetTimer(accessor_list_.at(0), 5, 0);

    // Drawing할 Ui object 등록
    ui_object_list_.push_back(std::make_unique<ExitUI>(ui_.getCurrentScreenSize()));

    // 최초에 한번 Draw 합니다.
    this->RenderProcess();
}
// ProcessResult EndState::UpdateProcess(std::chrono::duration<int64_t, std::nano> diff) {
ProcessResult EndState::UpdateProcess() {
    // auto n = std::chrono::duration_cast<std::chrono::milliseconds>(diff).count();
    // if (n < 0) return ProcessResult::kOut;
    // ready_milliseconds_ -= n;

    // timer 설정값 현재 5초 만큼 대기 후, Exit. Game 종료.
    if (accessor_list_.at(0).IsAlive() && !accessor_list_.at(0).IsRunning()) {
        return ProcessResult::kExit;
    }

    return ProcessResult::kNothing;
}
void EndState::RenderProcess() {
    for (auto itr = ui_object_list_.begin(); itr != ui_object_list_.end(); ++itr) {
        if (!(*itr)->IsChanged()) continue;

        ui_.Draw((*itr).get());
    }
}

void EndState::FinishProcess() {
    // 할당한 Object를 모두 해제합니다.
    ui_object_list_.erase(ui_object_list_.begin(), ui_object_list_.end());
}

/* GameState - MenuState Class ===================================================================================== */

MenuState::MenuState(GameManager& supervisor, UserData& user_player, UiHandler& ui, TimerHandler& timer)
    : GameState(supervisor, user_player, ui, timer), current_select_(kKeepPlaying) {}

void MenuState::MoveStateHandler(StateCode where) {
    this->FinishProcess();
    supervisor_.ChangeSelcet(where);
}

void MenuState::Initialize() {
}

// ProcessResult MenuState::InputProcess() {
//     return ProcessResult::kNothing;
// }

void MenuState::EnterProcess() {
    ui_.ClearScreen();

    // Drawing할 Ui object 등록
    ui_object_list_.push_back(std::make_unique<MenuUI>(ui_.getCurrentScreenSize()));

    // 최초에 한번 Draw 합니다.
    this->RenderProcess();

    current_select_ = 0;
    auto object_ptr = ui_object_list_.at(0).get();
    menu_accessor_ = dynamic_cast<MenuUI*>(object_ptr);
}
ProcessResult MenuState::UpdateProcess() {
    // ncurse Input
    auto menu_ptr = menu_accessor_->GetMenuAccessor();
    int input = ui_.getInput();
    switch (input) {
        case KEY_DOWN:
            ui_.ControlMenuDriver(menu_ptr, MenuRequest::DOWN);
            if (current_select_ != MenuItem::kExitPlay) current_select_++;
            menu_accessor_->UpdatePhysics();
            break;
        case KEY_UP:
            ui_.ControlMenuDriver(menu_ptr, MenuRequest::UP);
            if (current_select_ != MenuItem::kKeepPlaying) current_select_--;
            menu_accessor_->UpdatePhysics();
            break;
        case 10:  // Enter
        {
            switch (current_select_) {
                case MenuItem::kKeepPlaying:
                    break;
                case MenuItem::kCreateNewPlay:
                    break;
                case MenuItem::kLoadPreviousPlay:
                    break;
                case MenuItem::kPlayTogether:
                    break;
                case MenuItem::kSeeBoard:
                    break;
                case MenuItem::kExitPlay:
                    this->FinishProcess();
                    return ProcessResult::kExit;
                    break;
                default:
                    break;
            }
        } break;
        default:
            break;
    }

    return ProcessResult::kNothing;
}
void MenuState::RenderProcess() {
    for (auto itr = ui_object_list_.begin(); itr != ui_object_list_.end(); ++itr) {
        if (!(*itr)->IsChanged()) continue;

        ui_.Draw((*itr).get());
    }
}

void MenuState::FinishProcess() {
    keypad(menu_accessor_->GetMenuWinAccessor(), FALSE);
    // 할당한 Object를 모두 해제합니다.
    ui_object_list_.erase(ui_object_list_.begin(), ui_object_list_.end());
}

/* GameManager Class ===================================================================================== */
constexpr YX GameManager::game_size_;
GameManager::GameManager(UiHandler* ui_handler, timer::TimerHandler* timer_handler, int select_state)
    : ui_handler_(ui_handler), timer_handler_(timer_handler), select_state_(select_state) {
}

// TODO: exception condtion's needed
GameManager::~GameManager() {
    for (auto i = game_state_.begin(); i != game_state_.end(); ++i) i->release();
}

void GameManager::ChangeSelcet(StateCode where) {
    select_state_ = static_cast<int>(where);
}

bool GameManager::CheckGameState() const {
    // for (int i = 0; i < game_state_.size(); ++i) { //TODO: Original
    for (int i = 0; i < 2; ++i) {  // TODO: 개발용 임시, EndState까지
        if (game_state_[i] == nullptr) return false;
    }

    return true;
}

bool GameManager::CheckScreenSize(YX& screen_size) {
    if (screen_size.y < game_size_.y || screen_size.x < game_size_.x)
        return false;
    else
        return true;
}

YX GameManager::getNeededScreenSize() {
    return GameManager::game_size_;
}

void GameManager::LoadPreviousUserData() {
}

void GameManager::Initialize() {
    // Ui driver는 생성자 단계에서 받습니다.
    if (ui_handler_ == nullptr) throw std::runtime_error(std::string("E001 : UI Driver 없음"));
    if (timer_handler_ == nullptr) throw std::runtime_error(std::string("EE008 : TimerHandler 등록 안됨"));

    // GameState Initalizing
    for (auto i = game_state_.begin(); i != game_state_.end(); ++i) (*i) = nullptr;  // std::move(nullptr);
    // GameState는 GameManager가 소유합니다.
    game_state_[0] = std::make_unique<StartState>(*this, *(player_.get()), *(ui_handler_), *(timer_handler_));
    game_state_[1] = std::make_unique<EndState>(*this, *(player_.get()), *(ui_handler_), *(timer_handler_));
    game_state_[2] = std::make_unique<MenuState>(*this, *(player_.get()), *(ui_handler_), *(timer_handler_));
    // game_state_[3] = std::make_unique<TemperaryStopState>(*this, *(player_.get()), *(ui_));
    // game_state_[4] = std::make_unique<SoloPlayState>(*this, *(player_.get()), *(ui_));
    // game_state_[5] = std::make_unique<DuoPlayState>(*this, *(player_.get()), *(ui_));
    // game_state_[6] = std::make_unique<MultiPlayState>(*this, *(player_.get()), *(ui_));
    // game_state_[7] = std::make_unique<SettingState>(*this, *(player_.get()), *(ui_));
    // game_state_[8] = std::make_unique<CreditState>(*this, *(player_.get()), *(ui_));

    // 생성 Check
    if (!CheckGameState()) throw std::runtime_error(std::string("E002 : GameState isn't Loaded"));

    // 각 GameState Initializing
    // for (int i = 0; i < sizeof(game_state_) / sizeof(std::unique_ptr<GameState>); ++i) //Original
    for (int i = 0; i != 2; ++i)  // TODO: 임시.
        game_state_[i]->Initialize();

    /** 화면 크기 Check, 게임 실행에 필요한 크기는 다음과 같습니다.
     *  y   : 46
     *  x : 160
     */
    YX screen_size;
    screen_size = ui_handler_->getCurrentScreenSize();
    if (!CheckScreenSize(screen_size)) throw std::runtime_error(std::string("E003 : Terminal 크기 부족"));

    /*TODO: Sound 등록 필요 */

    /* FrameTime Object 등록 */
    // frame_time_object_ = std::make_unique<FramePerSecondUI>(game_size_.y - 2, 1);
}

void GameManager::Run() {
    // TestCode //TODO: 나중에 지울것
    // GameManagerTestCode();
    // GameManagerTestThreadManager();

    game_state_.at(select_state_)->EnterProcess();
    // std::chrono::time_point<std::chrono::high_resolution_clock> past = std::chrono::time_point<std::chrono::high_resolution_clock>::max();

    while (true) {
        ProcessResult n = kNothing;

        // std::chrono::time_point<std::chrono::high_resolution_clock> present = std::chrono::high_resolution_clock::now();
        // auto diff = present - past;
        // past = present;

        // GameManagerTestTimer(*this, diff, present, past);  // TestCode

        /* 21.11.14 Command Pattern을 사용하지 않기로 했으므로, 현재 시점에선 InputProccess는 필요가 없다. */
        // if ((n = game_state_.at(select_state_)->InputProcess()) == ProcessResult::kNothing) {
        if ((n = game_state_.at(select_state_)->UpdateProcess()) == ProcessResult::kNothing) {
            game_state_.at(select_state_)->RenderProcess();
        }
        //}

        switch (n) {
            case kNothing:
                // Normal Excution
                break;
            case kChangeState:
                game_state_.at(select_state_)->EnterProcess();
                break;
            case kOut:
                // Not error, but just away
                break;
            case kExit:
                return;  // finish(정상 종료)
                break;

            default:
                throw std::runtime_error(std::string("E004 : 알 수 없는 입력"));
                break;
        }
    }
}

/* TestCode =========================================================================================== */
void GameManagerTestCode(void) {
    attrset(A_ITALIC);
    mvprintw(5, 0, "attrset ITALIC");
    refresh();
    sleep(5);
    attrset(A_NORMAL);
}

void GameManagerTestThreadManager(void) {
    int start_t1 = 6;
    int start_t2 = 14;
    std::thread t(GameManagerTestThread, &start_t1);
    std::thread t2(GameManagerTestThread, &start_t2);

    t.detach();
    t2.detach();
}

void GameManagerTestThread(int* start_t) {
    for (int i = *start_t; i != (*start_t) + 4; ++i) {
        mvprintw(i, 0, "%d gogo", i);
        refresh();
        sleep(3);
    }
}
void GameManagerTestTimer(GameManager& G, std::chrono::duration<int64_t, std::nano> diff, std::chrono::time_point<std::chrono::high_resolution_clock> present, std::chrono::time_point<std::chrono::high_resolution_clock> past) {
    static int64_t t = 5000;
    static WINDOW* win_ = newwin(10, 50, 30, 2);

    int64_t k1 = std::chrono::duration_cast<std::chrono::nanoseconds>(diff).count();
    int64_t k2 = std::chrono::duration_cast<std::chrono::microseconds>(diff).count();
    int64_t k3 = std::chrono::duration_cast<std::chrono::milliseconds>(diff).count();
    int64_t k4 = std::chrono::duration_cast<std::chrono::seconds>(diff).count();
    std::time_t time1 = std::chrono::high_resolution_clock::to_time_t(present);
    std::time_t time2 = std::chrono::high_resolution_clock::to_time_t(past);
    char* date1 = ctime(&time1);
    char* date2 = ctime(&time2);
    clear();
    mvwprintw(win_, 0, 0, "%d", k1);
    mvwprintw(win_, 1, 0, "%d", k2);
    mvwprintw(win_, 2, 0, "%d", k3);
    mvwprintw(win_, 3, 0, "%d", k4);
    mvwprintw(win_, 4, 0, "%s", date1);
    mvwprintw(win_, 5, 0, "%s", date2);
    mvwprintw(win_, 6, 0, "%lld", static_cast<int64_t>(time1));
    mvwprintw(win_, 7, 0, "%lld", static_cast<int64_t>(time2));

    wrefresh(win_);
    usleep(100);
    bool flag = false;
    if (!flag) {
        mvwprintw(win_, 9, 0, "%lld", k4);
        wrefresh(win_);
        flag = true;
    }
    // if (k < 0) return;
    // t -= k;
    // if (t < 0) {
    //     //   시간경과 표시 for develop
    //     // G.frame_time_object_->UpdateCurrentTime(&present);
    //     // if (G.frame_time_object_->IsChanged()) G.ui_->Draw(G.frame_time_object_.get());
    //     t = 5000;
    // }

    // mvwprintw(win_, 0, 0, "%ld %5ld", k, t);
    // wrefresh(win_);
    // usleep(1);
}

}  // namespace cli_tetris