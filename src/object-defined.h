#ifndef CLI_TETRIS_OBJECT_DEFINED_H_
#define CLI_TETRIS_OBJECT_DEFINED_H_

#include <chrono>
#include <ui-handler.h>

extern "C" {
#include <ncurses.h>
#include <menu.h>
#include <panel.h>
}

namespace cli_tetris {

using Move = enum Move { kNothing = 0,
                         kUP,
                         kDown,
                         kLeft,
                         kRight };

using BlockType = enum BlockType {
    I = 1,
    J = 2,
    L = 3,
    T = 4,
    O = 5,
    Z = 6,
    S = 7
};

using YX = struct YX {
    int y;
    int x;
    constexpr YX(const int& y, const int& x) : y(y), x(x) {}
    constexpr YX(const YX& obj) : y(obj.y), x(obj.x) {}
    constexpr YX() : y(0), x(0) {}
    /*
    YX(std::initializer_list<int> l){
        this->y = *(l.begin());
        this->x = *(l.begin() + 1);
    }
    */
};

/* Object Class ===================================================================================== */
class Object {
   private:
    bool is_changed;

   protected:
    YX start_pos_;

   protected:
    Object(const YX& start_pos);
    Object();
    Object(const Object& object) = delete;

    YX getObjectPos() const;
    void setObjectPos(int y, int x);

    /**
     * 내부 상태가 변했는지를 설정하는 method
     * UpdatePyhiscs 와 UpdateRendering 내부에서 사용합니다.
     */
    void setIsChanged(bool changed);

   public:
    virtual ~Object(){};

    bool IsChanged() const;
    virtual void UpdatePhysics() = 0;
    virtual void UpdateRendering() = 0;
};
/* TetrisBlock Class ===================================================================================== */

class TetrisBlock : public Object {
   private:
    BlockType type_;
    std::array<YX, 16> real_block_shape_;
    int direction_;

   private:
    void DecideShape(int direction);
    void RandomiseDirection(int random_number_of_4);

   public:
    /** Constructor
     *  @param random_number_of_4, 0 이면 direction_ = 0 인 shape 생성, 1~4의 random number이면 해당하는 direction_ 결정후 shape 생성
     */
    TetrisBlock(const YX& start_pos, const BlockType& block_type, int random_number_of_4);
    ~TetrisBlock();

    // Object Abstract
    void UpdatePhysics() override;
    void UpdateRendering() override;

    // Custom method
    void CommandChangeDirection();
    void CommandFall();
    const std::array<YX, 16>& getRealBlockPosition() const;

    static std::array<YX, 16> ForcastChangeDirection(const TetrisBlock& object);
    static std::array<YX, 16> ForcastMoving(const TetrisBlock& object, const Move& move);
};

/* UI Class ===================================================================================== */
/**
 *  ncurse newwin() 을 통해 window 생성 및 Variable 소유 class
 */
class UI : public Object {
   protected:
    WINDOW* win_;
    YX win_size_;
    const YX currnet_screen_size_;

   protected:
    UI(const YX& currnet_screen_size, const YX& start_pos, const YX& win_size);
    // Start position이 상속한 class의 생성자에서 사전에 계산함을 됨을 위함
    UI(const YX& currnet_screen_size);

   public:
    virtual ~UI();

    // Object Class Abstract
    virtual void UpdatePhysics() = 0;
    virtual void UpdateRendering() = 0;
};

/* FramePerSecond Class ===================================================================================== */
// class FramePerSecondUI : public UI {
//    protected:
//    private:
//     std::chrono::time_point<std::chrono::high_resolution_clock> present_;
//     std::chrono::duration<int64_t, std::nano> diff_;

//    public:
//     FramePerSecondUI(YX start_pos);
//     FramePerSecondUI(int start_y, int start_x);
//     ~FramePerSecondUI();
//     void UpdatePhysics() override;
//     void UpdateRendering() override;

//     void UpdateCurrentTime(std::chrono::time_point<std::chrono::high_resolution_clock>* present);
//     void UpdateDifferTime(std::chrono::duration<int64_t, std::nano>* diff);
// };

/* StandbyUI Class ===================================================================================== */
class StandbyUI : public UI {
   public:
    //사전에 계산된 생성자
    StandbyUI(const YX& currnet_screen_size);
    ~StandbyUI() = default;

    // Object Abstract
    void UpdatePhysics() override;
    void UpdateRendering() override;
};

/* ExitUI Class ===================================================================================== */
class ExitUI : public UI {
   public:
    ExitUI(const YX& currnet_screen_size);
    ~ExitUI() = default;

    // Object Abstract
    void UpdatePhysics() override;
    void UpdateRendering() override;
};

/* MenuUI Class ===================================================================================== */
class MenuUI : public UI {
   protected:
    // For Menu
    MENU* menu_;
    WINDOW* menu_win_;
    ITEM** items_;
    char item_names[10][30];
    int item_names_num;
    // Entire UI
    using StructureData = struct StructureData {
        bool hide;
    };
    PANEL* ui_panel_[3];
    StructureData ui_panel_data_[3];

   public:
    MenuUI(const YX& currnet_screen_size);
    ~MenuUI();

    // Object Abstract
    void UpdatePhysics() override;
    void UpdateRendering() override;

    MENU* GetMenuAccessor() const;
    WINDOW* GetMenuWinAccessor() const;
};

/* FrameUI46X160 Class ===================================================================================== */
class FrameUI46X160 : public UI {
   public:
    FrameUI46X160(const YX& currnet_screen_size);
    ~FrameUI46X160();

    // Object Abstract
    void UpdatePhysics() override;
    void UpdateRendering() override;
};

/* TetrisBoardUI Class ===================================================================================== */
class TetrisBoardUI : public UI {
   private:
    YX relative_start_pos_;
    std::array<std::array<int, 21>, 41> board_;
    YX block_entry_point_;

   private:
   public:
    TetrisBoardUI(const YX& currnet_screen_size, const YX& offset);
    ~TetrisBoardUI();

    // Object Abstract
    void UpdatePhysics() override;
    void UpdateRendering() override;

    std::array<std::array<int, 21>, 41>* getTetrisBoard();
    const YX& getBlockEntryPoint() const;
};

/* TopBoardUI Class ===================================================================================== */
class TopBoardUI : public UI {
   private:
    YX relative_start_pos_;

   public:
    TopBoardUI(const YX& currnet_screen_size, const YX& offset);
    ~TopBoardUI();

    // Object Abstract
    void UpdatePhysics() override;
    void UpdateRendering() override;
};

/* ScoreBoardUI Class ===================================================================================== */
class ScoreBoardUI : public UI {
   private:
    YX relative_start_pos_;

   public:
    ScoreBoardUI(const YX& currnet_screen_size, const YX& offset);
    ~ScoreBoardUI();

    // Object Abstract
    void UpdatePhysics() override;
    void UpdateRendering() override;
};
/* NextTetrisBoardUI Class ===================================================================================== */
class NextTetrisBoardUI : public UI {
   private:
    YX relative_start_pos_;

   public:
    NextTetrisBoardUI(const YX& currnet_screen_size, const YX& offset);
    ~NextTetrisBoardUI();

    // Object Abstract
    void UpdatePhysics() override;
    void UpdateRendering() override;
};
/* LevelBoardUI Class ===================================================================================== */
class LevelBoardUI : public UI {
   private:
    YX relative_start_pos_;

   public:
    LevelBoardUI(const YX& currnet_screen_size, const YX& offset);
    ~LevelBoardUI();

    // Object Abstract
    void UpdatePhysics() override;
    void UpdateRendering() override;
};
/* InformBoardUI Class ===================================================================================== */
class InformBoardUI : public UI {
   private:
    YX relative_start_pos_;

   public:
    InformBoardUI(const YX& currnet_screen_size, const YX& offset);
    ~InformBoardUI();

    // Object Abstract
    void UpdatePhysics() override;
    void UpdateRendering() override;
};

}  // namespace cli_tetris

#endif  // CLI_TETRIS_GAME_OBJECT_DEFINED_H_