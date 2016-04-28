# include <scale.h>
# define NK_INCLUDE_FIXED_TYPES
# define NK_INCLUDE_STANDARD_IO
# define NK_INCLUDE_DEFAULT_ALLOCATOR
# define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
# define NK_INCLUDE_FONT_BAKING
# define NK_INCLUDE_DEFAULT_FONT
# define NK_IMPLEMENTATION
# include <nuklear.h>
#ifdef __APPLE__
  #define NK_SHADER_VERSION "#version 150\n"
#else
  #define NK_SHADER_VERSION "#version 300 es\n"
#endif
# define MAX_VERTEX_MEMORY 512 * 1024
# define MAX_ELEMENT_MEMORY 128 * 1024
# define UNUSED(a) (void)a



struct		device {
	struct nk_buffer			cmds;
	struct nk_draw_null_texture	null;
	GLuint						vbo, vao, ebo;
	GLuint						prog;
	GLuint						vert_shdr;
	GLuint						frag_shdr;
	GLint						attrib_pos;
	GLint						attrib_uv;
	GLint						attrib_col;
	GLint						uniform_tex;
	GLint						uniform_proj;
	GLuint						font_tex;
};

static void text_input(GLFWwindow *win, unsigned int codepoint) {
	nk_input_unicode((struct nk_context*)glfwGetWindowUserPointer(win), codepoint);
}

static void scroll_input(GLFWwindow *win, double _, double yoff) {
	UNUSED(_);
	nk_input_scroll((struct nk_context*)glfwGetWindowUserPointer(win), (float)yoff);
}

static void device_init(struct device *dev) {
	GLint status;
	static const GLchar *vertex_shader =
		NK_SHADER_VERSION
		"uniform mat4 ProjMtx;\n"
		"in vec2 Position;\n"
		"in vec2 TexCoord;\n"
		"in vec4 Color;\n"
		"out vec2 Frag_UV;\n"
		"out vec4 Frag_Color;\n"
		"void main() {\n"
		"   Frag_UV = TexCoord;\n"
		"   Frag_Color = Color;\n"
		"   gl_Position = ProjMtx * vec4(Position.xy, 0, 1);\n"
		"}\n";
	static const GLchar *fragment_shader =
		NK_SHADER_VERSION
		"precision mediump float;\n"
		"uniform sampler2D Texture;\n"
		"in vec2 Frag_UV;\n"
		"in vec4 Frag_Color;\n"
		"out vec4 Out_Color;\n"
		"void main(){\n"
		"   Out_Color = Frag_Color * texture(Texture, Frag_UV.st);\n"
		"}\n";

	nk_buffer_init_default(&dev->cmds);
	dev->prog = glCreateProgram();
	dev->vert_shdr = glCreateShader(GL_VERTEX_SHADER);
	dev->frag_shdr = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(dev->vert_shdr, 1, &vertex_shader, 0);
	glShaderSource(dev->frag_shdr, 1, &fragment_shader, 0);
	glCompileShader(dev->vert_shdr);
	glCompileShader(dev->frag_shdr);
	glGetShaderiv(dev->vert_shdr, GL_COMPILE_STATUS, &status);
	assert(status == GL_TRUE);
	glGetShaderiv(dev->frag_shdr, GL_COMPILE_STATUS, &status);
	assert(status == GL_TRUE);
	glAttachShader(dev->prog, dev->vert_shdr);
	glAttachShader(dev->prog, dev->frag_shdr);
	glLinkProgram(dev->prog);
	glGetProgramiv(dev->prog, GL_LINK_STATUS, &status);
	assert(status == GL_TRUE);

	dev->uniform_tex = glGetUniformLocation(dev->prog, "Texture");
	dev->uniform_proj = glGetUniformLocation(dev->prog, "ProjMtx");
	dev->attrib_pos = glGetAttribLocation(dev->prog, "Position");
	dev->attrib_uv = glGetAttribLocation(dev->prog, "TexCoord");
	dev->attrib_col = glGetAttribLocation(dev->prog, "Color");

	{
		/* buffer setup */
		GLsizei vs = sizeof(struct nk_draw_vertex);
		size_t vp = offsetof(struct nk_draw_vertex, position);
		size_t vt = offsetof(struct nk_draw_vertex, uv);
		size_t vc = offsetof(struct nk_draw_vertex, col);

		glGenBuffers(1, &dev->vbo);
		glGenBuffers(1, &dev->ebo);
		glGenVertexArrays(1, &dev->vao);
		glBindVertexArray(dev->vao);
		glBindBuffer(GL_ARRAY_BUFFER, dev->vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dev->ebo);
		glEnableVertexAttribArray((GLuint)dev->attrib_pos);
		glEnableVertexAttribArray((GLuint)dev->attrib_uv);
		glEnableVertexAttribArray((GLuint)dev->attrib_col);
		glVertexAttribPointer((GLuint)dev->attrib_pos, 2, GL_FLOAT, GL_FALSE, vs, (void*)vp);
		glVertexAttribPointer((GLuint)dev->attrib_uv, 2, GL_FLOAT, GL_FALSE, vs, (void*)vt);
		glVertexAttribPointer((GLuint)dev->attrib_col, 4, GL_UNSIGNED_BYTE, GL_TRUE, vs, (void*)vc);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

static void device_upload_atlas(struct device *dev, const void *image, int width, int height) {
	glGenTextures(1, &dev->font_tex);
	glBindTexture(GL_TEXTURE_2D, dev->font_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)width, (GLsizei)height, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, image);
}

static void device_shutdown(struct device *dev) {
	glDetachShader(dev->prog, dev->vert_shdr);
	glDetachShader(dev->prog, dev->frag_shdr);
	glDeleteShader(dev->vert_shdr);
	glDeleteShader(dev->frag_shdr);
	glDeleteProgram(dev->prog);
	glDeleteTextures(1, &dev->font_tex);
	glDeleteBuffers(1, &dev->vbo);
	glDeleteBuffers(1, &dev->ebo);
	nk_buffer_free(&dev->cmds);
}

static void device_draw(struct device *dev, struct nk_context *ctx, int width, int height,
		enum nk_anti_aliasing AA) {
	GLint last_prog, last_tex;
	GLint last_ebo, last_vbo, last_vao;
	GLfloat ortho[4][4] = {
		{2.0f, 0.0f, 0.0f, 0.0f},
		{0.0f,-2.0f, 0.0f, 0.0f},
		{0.0f, 0.0f,-1.0f, 0.0f},
		{-1.0f,1.0f, 0.0f, 1.0f},
	};
	ortho[0][0] /= (GLfloat)width;
	ortho[1][1] /= (GLfloat)height;

	/* save previous opengl state */
	glGetIntegerv(GL_CURRENT_PROGRAM, &last_prog);
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_tex);
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_vao);
	glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &last_ebo);
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vbo);

	/* setup global state */
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_SCISSOR_TEST);
	glActiveTexture(GL_TEXTURE0);

	/* setup program */
	glUseProgram(dev->prog);
	glUniform1i(dev->uniform_tex, 0);
	glUniformMatrix4fv(dev->uniform_proj, 1, GL_FALSE, &ortho[0][0]);
	{
		/* convert from command queue into draw list and draw to screen */
		const struct nk_draw_command *cmd;
		void *vertices, *elements;
		const nk_draw_index *offset = NULL;

		/* allocate vertex and element buffer */
		glBindVertexArray(dev->vao);
		glBindBuffer(GL_ARRAY_BUFFER, dev->vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dev->ebo);

		glBufferData(GL_ARRAY_BUFFER, MAX_VERTEX_MEMORY, NULL, GL_STREAM_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_ELEMENT_MEMORY, NULL, GL_STREAM_DRAW);

		/* load draw vertices & elements directly into vertex + element buffer */
		vertices = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		elements = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
		{
			/* fill converting configuration */
			struct nk_convert_config config;
			memset(&config, 0, sizeof(config));
			config.global_alpha = 1.0f;
			config.shape_AA = AA;
			config.line_AA = AA;
			config.circle_segment_count = 22;
			config.curve_segment_count = 22;
			config.arc_segment_count = 22;
			config.null = dev->null;

			/* setup buffers to load vertices and elements */
			{struct nk_buffer vbuf, ebuf;
				nk_buffer_init_fixed(&vbuf, vertices, MAX_VERTEX_MEMORY);
				nk_buffer_init_fixed(&ebuf, elements, MAX_ELEMENT_MEMORY);
				nk_convert(ctx, &dev->cmds, &vbuf, &ebuf, &config);}
		}
		glUnmapBuffer(GL_ARRAY_BUFFER);
		glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

		/* iterate over and execute each draw command */
		nk_draw_foreach(cmd, ctx, &dev->cmds) {
			if (!cmd->elem_count) continue;
			glBindTexture(GL_TEXTURE_2D, (GLuint)cmd->texture.id);
			glScissor((GLint)cmd->clip_rect.x,
					height - (GLint)(cmd->clip_rect.y + cmd->clip_rect.h),
					(GLint)cmd->clip_rect.w, (GLint)cmd->clip_rect.h);
			glDrawElements(GL_TRIANGLES, (GLsizei)cmd->elem_count, GL_UNSIGNED_SHORT, offset);
			offset += cmd->elem_count;
		}
		nk_clear(ctx);
	}

	/* restore old state */
	glUseProgram((GLuint)last_prog);
	glBindTexture(GL_TEXTURE_2D, (GLuint)last_tex);
	glBindBuffer(GL_ARRAY_BUFFER, (GLuint)last_vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (GLuint)last_ebo);
	glBindVertexArray((GLuint)last_vao);
	glDisable(GL_SCISSOR_TEST);
}

static void error_callback(int e, const char *d){
	ERROR("GLFW Error %d: %s\n", e, d);
}

static GLFWwindow	*init(struct nk_context *ctx, struct device *device) {
	struct nk_font			*font;
	struct nk_font_atlas	atlas;
	GLFWwindow				*win = 0x0;
	const void				*image;
	int						w, h;

	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
		ERROR("Failed to init GLFW!\n");
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	win = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "42_scale", NULL, NULL);
	glfwMakeContextCurrent(win);
	glfwSetWindowUserPointer(win, ctx);
	glfwSetCharCallback(win, text_input);
	glfwSetScrollCallback(win, scroll_input);
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	glewExperimental = 1;
	if (glewInit() != GLEW_OK) {
		ERROR("Failed to setup GLEW\n");
	}
    device_init(device);
	nk_font_atlas_init_default(&atlas);
	nk_font_atlas_begin(&atlas);
	font = nk_font_atlas_add_default(&atlas, 13.0f, NULL);
	image = nk_font_atlas_bake(&atlas, &w, &h, NK_FONT_ATLAS_RGBA32);
	device_upload_atlas(device, image, w, h);
	nk_font_atlas_end(&atlas, nk_handle_id((int)device->font_tex), &device->null);
	nk_init_default(ctx, &(font->handle));
	return win;
}

void	handle_input(struct nk_context *ctx, GLFWwindow *win) {
	double x, y;

	nk_input_begin(ctx);
	glfwPollEvents();
	nk_input_key(ctx, NK_KEY_DEL, glfwGetKey(win, GLFW_KEY_DELETE) == GLFW_PRESS);
	nk_input_key(ctx, NK_KEY_ENTER, glfwGetKey(win, GLFW_KEY_ENTER) == GLFW_PRESS);
	nk_input_key(ctx, NK_KEY_TAB, glfwGetKey(win, GLFW_KEY_TAB) == GLFW_PRESS);
	nk_input_key(ctx, NK_KEY_BACKSPACE, glfwGetKey(win, GLFW_KEY_BACKSPACE) == GLFW_PRESS);
	nk_input_key(ctx, NK_KEY_LEFT, glfwGetKey(win, GLFW_KEY_LEFT) == GLFW_PRESS);
	nk_input_key(ctx, NK_KEY_RIGHT, glfwGetKey(win, GLFW_KEY_RIGHT) == GLFW_PRESS);
	nk_input_key(ctx, NK_KEY_UP, glfwGetKey(win, GLFW_KEY_UP) == GLFW_PRESS);
	nk_input_key(ctx, NK_KEY_DOWN, glfwGetKey(win, GLFW_KEY_DOWN) == GLFW_PRESS);
	if (glfwGetKey(win, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
			glfwGetKey(win, GLFW_KEY_RIGHT_CONTROL)) {
		nk_input_key(ctx, NK_KEY_COPY, glfwGetKey(win, GLFW_KEY_C) == GLFW_PRESS);
		nk_input_key(ctx, NK_KEY_PASTE, glfwGetKey(win, GLFW_KEY_P) == GLFW_PRESS);
		nk_input_key(ctx, NK_KEY_CUT, glfwGetKey(win, GLFW_KEY_X) == GLFW_PRESS);
		nk_input_key(ctx, NK_KEY_CUT, glfwGetKey(win, GLFW_KEY_E) == GLFW_PRESS);
		nk_input_key(ctx, NK_KEY_SHIFT, 1);
	} else {
		nk_input_key(ctx, NK_KEY_COPY, 0);
		nk_input_key(ctx, NK_KEY_PASTE, 0);
		nk_input_key(ctx, NK_KEY_CUT, 0);
		nk_input_key(ctx, NK_KEY_SHIFT, 0);
	}
	glfwGetCursorPos(win, &x, &y);
	nk_input_motion(ctx, (int)x, (int)y);
	nk_input_button(ctx, NK_BUTTON_LEFT, (int)x, (int)y, glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
	nk_input_button(ctx, NK_BUTTON_MIDDLE, (int)x, (int)y, glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS);
	nk_input_button(ctx, NK_BUTTON_RIGHT, (int)x, (int)y, glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);
	nk_input_end(ctx);
}

void	handle_window(scale *s, struct nk_context *ctx) {
	struct nk_panel		layout, menu;
	static const float	ratio[] = {120, 150}, ratio2[] = {300, 150};
	const char				*lang[] = {"French", "English", "LANG_RU"};
	int					w_flag = NK_WINDOW_TITLE | NK_WINDOW_BORDER |
					NK_WINDOW_CLOSABLE | NK_WINDOW_SCALABLE | NK_WINDOW_MOVABLE;

	if (nk_begin(ctx, &layout, "Main", nk_rect(10, 10, 600, 900), w_flag)) {
		nk_menubar_begin(ctx);
		nk_layout_row_begin(ctx, NK_STATIC, 25, 2);
		nk_layout_row_push(ctx, 45);
		if (nk_menu_begin_label(ctx, &menu, "Menu", NK_TEXT_LEFT, 120)) {
			nk_layout_row_dynamic(ctx, 25, 1);
			if (nk_menu_item_label(ctx, "Save", NK_TEXT_LEFT)) {
				save_scale(s);
			}
			if (nk_menu_item_label(ctx, "Quit", NK_TEXT_LEFT)) {
				_exit(0);
			}
			nk_menu_end(ctx);
		}
		nk_menubar_end(ctx);
		nk_layout_row(ctx, NK_STATIC, 25, 2, ratio);
		nk_label(ctx, "Subject name", NK_TEXT_LEFT);
		nk_edit_string(ctx, NK_EDIT_FIELD, s->name.buf, &s->name.len, 64, nk_filter_default);
		nk_label(ctx, "Language", NK_TEXT_LEFT);
		s->lang.val = nk_combo(ctx, lang, 3, s->lang.val, 25);
		nk_label(ctx, "Comment", NK_TEXT_LEFT);
		nk_edit_string(ctx, NK_EDIT_FIELD, s->comment.buf, &s->comment.len, 64, nk_filter_default);
		nk_label(ctx, "Correction", NK_TEXT_LEFT);
		nk_property_int(ctx, "Number:", 0, &s->correction_n.val, 100.0f, 1, 1);
		nk_label(ctx, "Correction", NK_TEXT_LEFT);
		nk_property_int(ctx, "Duration:", 0, &s->duration.val, 100.0f, 1, 1);
		nk_label(ctx, "Introduction", NK_TEXT_LEFT);
		nk_layout_row_static(ctx, count_lines(s->intro.buf), 550, 1);
		nk_edit_string(ctx, NK_EDIT_BOX, s->intro.buf, &s->intro.len, 2024,  nk_filter_ascii);
		nk_layout_row(ctx, NK_STATIC, 25, 2, ratio);
		nk_label(ctx, "Guidelines", NK_TEXT_LEFT);
		nk_layout_row_static(ctx, count_lines(s->guidelines.buf), 550, 1);
		nk_edit_string(ctx, NK_EDIT_BOX | NK_COLOR_EDIT_CURSOR, s->guidelines.buf, &s->guidelines.len, 2024,  nk_filter_ascii);
		nk_layout_row_end(ctx);
	}
	nk_end(ctx);

	nk_flags		active;
	static char		*new_section = 0x0, new_question[30][64];
	static int		ns_len = 0, nq_len[30], nq_active[30], sk_add[64] = {0};
	const char				*rating[] = {"Boolean", "Multi"};
	const char				*kind[] = {"Standard", "Bonus"};
	const char				*skills[] = {"Adaptation & Creativity", "Algorithms & AI", "Company experience", "DB & Data",
								"Functionnal programming", "Graphics", "Group & interpersonal", "Imperative programming",
								"Network & system administration", "Object-oriented programming",
								"Organization", "Parallel computing", "Rigor", "Security", "Technology integration", "Unix", "Web"};
	int				j = 0, q = 0, sk_totals_sd[30] = {0}, sk_totals_bs[30] = {0};

	if (!new_section) {
		new_section = malloc(64);
	}
	if (nk_begin(ctx, &layout, "Sections", nk_rect(620, 10, 600, 900), w_flag)) {
		nk_layout_row(ctx, NK_STATIC, 25, 2, ratio);
		active = nk_edit_string(ctx, NK_EDIT_FIELD|NK_EDIT_SIG_ENTER, new_section, &ns_len, 64,  nk_filter_ascii);
		if (nk_button_label(ctx, "Add a Section", NK_BUTTON_DEFAULT) || (active & NK_EDIT_COMMITED)) {
			new_section[ns_len] = 0x0;
			add_section(s->sections, new_section);
			new_section = "";
			ns_len = 0;
		}
		for (scale_sections *it = s->sections; it; it = it->next, j++) {
			if (nk_tree_push(ctx, NK_TREE_NODE, it->name.buf, NK_MINIMIZED)) {
				nk_layout_row(ctx, NK_STATIC, 25, 2, ratio);
				nk_label(ctx, "Add a Question", NK_TEXT_LEFT);
				nq_active[j] = nk_edit_string(ctx, NK_EDIT_FIELD | NK_EDIT_SIG_ENTER, new_question[j], &nq_len[j], 64, nk_filter_ascii);
				nk_label(ctx, "Description", NK_TEXT_LEFT);
				nk_edit_string(ctx, NK_EDIT_FIELD, it->description.buf, &it->description.len, 64, nk_filter_default);
				nk_layout_row_end(ctx);
				for (scale_questions *it2 = it->questions; it2; it2 = it2->next, q++) {
					if (nk_tree_push(ctx, NK_TREE_NODE, it2->name.buf, NK_MINIMIZED)) {
						nk_label(ctx, "Guidelines", NK_TEXT_LEFT);
						nk_layout_row_static(ctx, count_lines(it2->guidelines.buf), 550, 1);
						nk_edit_string(ctx, NK_EDIT_BOX, it2->guidelines.buf, &it2->guidelines.len, 2024,  nk_filter_ascii);
						nk_layout_row(ctx, NK_STATIC, 25, 2, ratio);
						nk_label(ctx, "Rating type", NK_TEXT_LEFT);
						it2->rating.val = nk_combo(ctx, rating, 2, it2->rating.val, 25);
						nk_label(ctx, "Kind", NK_TEXT_LEFT);
						it2->kind.val = nk_combo(ctx, kind, 2, it2->kind.val, 25);
						if (nk_tree_push(ctx, NK_TREE_NODE, "Skills", NK_MINIMIZED)) {
							nk_layout_row(ctx, NK_STATIC, 25, 2, ratio2);
							sk_add[q] = nk_combo(ctx, skills, 17, sk_add[q], 25);
							if (nk_button_label(ctx, "Add a Skill", NK_BUTTON_DEFAULT)) {
								add_skills(it2, sk_add[q], (char *)skills[q]);
								sk_add[q] = 0;
							}
							for (scale_skills *sk = it2->skills; sk; sk = sk->next) {
								nk_label(ctx, skills[sk->name.val], NK_TEXT_LEFT);
								nk_property_int(ctx, "Percent:", 0, &sk->percent.val, 100.0f, 1, 1);
							}
							nk_tree_pop(ctx);
						}
						nk_tree_pop(ctx);
					}
				}
				nk_tree_pop(ctx);
			}
		}
		for (j = 0; j < 30; j++) {
			if (nq_active[j] & NK_EDIT_COMMITED) {
				new_question[j][nq_len[j]] = 0x0;
				add_question(s->sections, new_question[j], j);
				strcpy(new_question[j], "");
				nq_len[j] = 0;
			}
		}
	}
	nk_end(ctx);
	for (scale_sections *it = s->sections; it; it = it->next, j++) {
		for (scale_questions *it2 = it->questions; it2; it2 = it2->next) {
				for (scale_skills *sk = it2->skills; sk; sk = sk->next) {
					if (it2->kind.val == R_STAND) {
						sk_totals_sd[sk->name.val] += sk->percent.val;
					} else {
						sk_totals_bs[sk->name.val] += sk->percent.val;
					}
			}
		}
	}
	if (nk_begin(ctx, &layout, "Skills totals", nk_rect(1250, 10, 600, 400), w_flag)) {
		if (nk_tree_push(ctx, NK_TREE_NODE, "Standard", NK_MAXIMIZED)) {
			nk_layout_row(ctx, NK_STATIC, 25, 2, ratio2);
			for (int j = 0; j < 30; j++) {
				if (sk_totals_sd[j] > 0) {
					nk_label(ctx, skills[j], NK_TEXT_LEFT);
					nk_property_int(ctx, "Total:", -10, &sk_totals_sd[j], 1000, 1, 1);
				}
				nk_layout_row_end(ctx);
			}
		}
		nk_tree_pop(ctx);
		if (nk_tree_push(ctx, NK_TREE_NODE, "Bonus", NK_MAXIMIZED)) {
			nk_layout_row(ctx, NK_STATIC, 25, 2, ratio2);
			for (int j = 0; j < 30; j++) {
				if (sk_totals_bs[j] > 0) {
					nk_label(ctx, skills[j], NK_TEXT_LEFT);
					nk_property_int(ctx, "Total:", -10, &sk_totals_bs[j], 1000, 1, 1);
				}
			}
		}
		nk_tree_pop(ctx);
	}
	nk_end(ctx);
}

void	window(scale *s) {
	GLFWwindow			*win = 0x0;
	int					width = 0, height = 0;
	struct nk_context	ctx;
	struct device		device;

	win = init(&ctx, &device);
	ctx.style.window.header.align = NK_HEADER_RIGHT;
	while (!glfwWindowShouldClose(win)) {
		handle_input(&ctx, win);
		handle_window(s, &ctx);
		glfwGetWindowSize(win, &width, &height);
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		device_draw(&device, &ctx, width, height, NK_ANTI_ALIASING_ON);
		glfwSwapBuffers(win);
	}
	nk_free(&ctx);
	device_shutdown(&device);
	glfwTerminate();

}
