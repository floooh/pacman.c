//------------------------------------------------------------------------------
//  pacman.c
//------------------------------------------------------------------------------
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_audio.h"
#include "sokol_time.h"
#include "sokol_glue.h"
#include <assert.h>

// the Pacman playfield is 28 tiles wide and 36 tiles high
#define NUM_TILES_X (28)
#define NUM_TILES_Y (36)
// each tile is 8x8 pixels
#define TILE_WIDTH  (8)
#define TILE_HEIGHT (8)
// up to 6 sprites can be displayed
#define NUM_SPRITES (6)
// max number of vertices required for rendering the playfield and sprites
#define MAX_VERTICES (((NUM_TILES_X * NUM_TILES_Y) + NUM_SPRITES) * 6)

typedef struct {
    float x, y;         // screen coords [0..1] as FLOAT2
    uint32_t data;      // texcoord and alpha
} gfx_vertex_t;

static struct {

    // the current input state
    struct {
        bool up;
        bool down;
        bool left;
        bool right;
        bool any;
    } input;

    // the gfx subsystem implements a simple tile+sprite renderer
    struct {
        sg_pass_action pass_action;
        sg_buffer vbuf;
        sg_image img;
        sg_pipeline pip;
        int num_vertices;
        uint8_t tiles[NUM_TILES_Y][NUM_TILES_X];
        gfx_vertex_t vertices[MAX_VERTICES];
    } gfx;
} state;

/*== APPLICATION ENTRY AND CALLBACKS =========================================*/
static void init(void);
static void frame(void);
static void cleanup(void);
static void input(const sapp_event*);

sapp_desc sokol_main(int argc, char* argv[]) {
    return (sapp_desc) {
        .init_cb = init,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .event_cb = input,
        .width = NUM_TILES_X * TILE_WIDTH * 2,
        .height = NUM_TILES_Y * TILE_HEIGHT * 2,
        .window_title = "pacman.c"
    };
}

static void gfx_init(void);
static void init(void) {
    // setup sokol libs
    sg_setup(&(sg_desc){
        .context = sapp_sgcontext()
    });
    stm_setup();
    saudio_setup(&(saudio_desc){ 0 });

    // initialize subsystems
    gfx_init();
}

static void gfx_draw(void);
static void frame(void) {
    gfx_draw();
}

static void input(const sapp_event* ev) {
    if ((ev->type == SAPP_EVENTTYPE_KEY_DOWN) || (ev->type == SAPP_EVENTTYPE_KEY_UP)) {
        bool btn_down = ev->type == SAPP_EVENTTYPE_KEY_DOWN;
        switch (ev->key_code) {
            case SAPP_KEYCODE_UP:
            case SAPP_KEYCODE_W:
                state.input.up = state.input.any = btn_down;
                break;
            case SAPP_KEYCODE_DOWN:
            case SAPP_KEYCODE_S:
                state.input.down = state.input.any = btn_down;
                break;
            case SAPP_KEYCODE_LEFT:
            case SAPP_KEYCODE_A:
                state.input.left = state.input.any = btn_down;
                break;
            case SAPP_KEYCODE_RIGHT:
            case SAPP_KEYCODE_D:
                state.input.right = state.input.any = btn_down;
                break;
            default:
                state.input.any = btn_down;
                break;
        }
    }
}

static void cleanup(void) {
    sg_shutdown();
    saudio_shutdown();
}

/*== GFX SUBSYSTEM ===========================================================*/
static void gfx_init(void) {

    // pass action for clearing the background to black
    state.gfx.pass_action = (sg_pass_action) {
        .colors[0] = { .action = SG_ACTION_CLEAR, .val = { 0.0f, 0.0f, 0.0f, 1.0f } }
    };

    // create a dynamic vertex buffer for the tile and sprite quads
    state.gfx.vbuf = sg_make_buffer(&(sg_buffer_desc){
        .type = SG_BUFFERTYPE_VERTEXBUFFER,
        .usage = SG_USAGE_STREAM,
        .size = sizeof(state.gfx.vertices),
    });

    // shader sources for all platforms (FIXME: should we use precompiled shader blobs instead?)
    const char* vs_src = 0;
    const char* fs_src = 0;
    switch (sg_query_backend()) {
        case SG_BACKEND_METAL_MACOS:
            vs_src = 
                "#include <metal_stdlib>\n"
                "using namespace metal;\n"
                "struct vs_in {\n"
                "  float4 pos [[attribute(0)]];\n"
                "  float4 data [[attribute(1)]];\n"
                "};\n"
                "struct vs_out {\n"
                "  float4 pos [[position]];\n"
                "  float4 data;\n"
                "};\n"
                "vertex vs_out _main(vs_in in [[stage_in]]) {\n"
                "  vs_out out;\n"
                "  out.pos = float4((in.pos.xy - 0.5) * float2(2.0, -2.0), 0.5, 1.0);\n"
                "  out.data = in.data;\n"
                "  return out;\n"
                "}\n";
            fs_src =
                "#include <metal_stdlib>\n"
                "using namespace metal;\n"
                "fragment float4 _main(float4 data [[stage_in]]) {\n"
                "  return float4(data.xy, 0.0, 1.0);\n"
                "}\n";
            break;
        default:
            assert(false);
    }

    // create pipeline and shader object
    state.gfx.pip = sg_make_pipeline(&(sg_pipeline_desc){
        .shader = sg_make_shader(&(sg_shader_desc){
            .vs.source = vs_src,
            .fs.source = fs_src
        }),
        .layout = {
            .attrs = {
                [0].format = SG_VERTEXFORMAT_FLOAT2,
                [1].format = SG_VERTEXFORMAT_UBYTE4N,
            }
        },
        .blend = {
            .enabled = true,
            .src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
            .dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_DST_ALPHA,
        }
    });
}

static inline void gfx_add_vertex(float x, float y, uint32_t data) {
    gfx_vertex_t* v = &state.gfx.vertices[state.gfx.num_vertices++];
    v->x = x;
    v->y = y;
    v->data = data;
}

static inline void gfx_add_tile_vertices(uint32_t x, uint32_t y, uint8_t tile_code) {
    const float dx = 1.0f / NUM_TILES_X;
    const float dy = 1.0f / NUM_TILES_Y;
    const float x0 = x * dx;
    const float x1 = x0 + dx;
    const float y0 = y * dy;
    const float y1 = y0 + dy;
    /*
        x0,y0
        +-----+
        | *   |
        |   * |
        +-----+
              x1,y1
    */
    gfx_add_vertex(x0, y0, 0xFFFFFFFF);
    gfx_add_vertex(x1, y0, 0xFFFFFFFF);
    gfx_add_vertex(x1, y1, 0xFFFFFFFF);
    gfx_add_vertex(x0, y0, 0xFF0000FF);
    gfx_add_vertex(x1, y1, 0xFF0000FF);
    gfx_add_vertex(x0, y1, 0xFF0000FF);
}

static void gfx_draw(void) {

    // update the playfield and sprite vertex buffer
    state.gfx.num_vertices = 0;
    for (uint32_t y = 0; y < NUM_TILES_Y; y++) {
        for (uint32_t x = 0; x < NUM_TILES_X; x++) {
            gfx_add_tile_vertices(x, y, state.gfx.tiles[y][x]);
        }
    }
    assert(state.gfx.num_vertices <= MAX_VERTICES);
    sg_update_buffer(state.gfx.vbuf, &state.gfx.vertices, state.gfx.num_vertices * sizeof(gfx_vertex_t));

    // render everything in a sokol-gfx pass
    const int canvas_width = sapp_width();
    const int canvas_height = sapp_height();
    sg_begin_default_pass(&state.gfx.pass_action, canvas_width, canvas_height);

    // force correct aspect ratio, with 5 pixels border
    const float canvas_aspect = (float)canvas_width / (float)canvas_height;
    const float playfield_aspect = (float)NUM_TILES_X / (float)NUM_TILES_Y;
    const int border = 5;
    int vp_x, vp_y, vp_w, vp_h;
    if (playfield_aspect < canvas_aspect) {
        vp_y = border;
        vp_h = canvas_height - (2 * border);
        vp_w = (int)(canvas_height * playfield_aspect) - (2 * border);
        vp_x = (canvas_width - vp_w) / 2;
    }
    else {
        vp_x = border;
        vp_w = canvas_width - 2 * border;
        vp_h = (int)(canvas_width / playfield_aspect) - (2 * border);
        vp_y = (canvas_height - vp_h) / 2;
    }
    sg_apply_viewport(vp_x, vp_y, vp_w, vp_h, true);
    sg_apply_pipeline(state.gfx.pip);
    sg_apply_bindings(&(sg_bindings){
        .vertex_buffers[0] = state.gfx.vbuf,
        .fs_images[0] = state.gfx.img,
    });
    sg_draw(0, state.gfx.num_vertices, 1);
    sg_end_pass();
    sg_commit();
}
