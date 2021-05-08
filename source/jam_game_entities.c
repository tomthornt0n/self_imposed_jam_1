typedef size_t ETT_Flags;
typedef enum
{
 ETT_Flags_playerControls = 1 << 0,
} ETT_Flags_ENUM;

typedef struct ETT_Entity ETT_Entity;
struct ETT_Entity
{
 ETT_Flags flags;
 Rect rect;
};