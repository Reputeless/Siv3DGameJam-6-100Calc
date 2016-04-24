# include <Siv3D.hpp>
# include <HamFramework.hpp>

struct GameData
{
	std::array<int32, 5> record = { 222, 333, 444, 555, 666 };
	int32 previousRecord = 666;
};

using MyApp = SceneManager<String, GameData>;

struct Title : MyApp::Scene
{
	void init() override
	{
		if (FileSystem::Exists(L"score.bin"))
		{
			BinaryReader(L"score.bin").read(m_data->record);
		}
	}

	void update() override
	{
		if (Input::MouseL.clicked || Input::AnyKeyClicked())
		{
			changeScene(L"Game", 500);
		}	
	}

	void draw() const override
	{
		RectF(1000).setCenter(Window::Center()).rotated(System::FrameCount() /  240.0).drawFrame(10, Color(207, 215, 225));
		RectF(1000).setCenter(Window::Center()).rotated(System::FrameCount() / -240.0).drawFrame(10, Color(207, 215, 225));

		FontAsset(L"Title")(L"百ます計算").drawCenter(640, 200, Color(50));

		for (auto i : step(m_data->record.size()))
		{
			FontAsset(L"Bold")(L"{} 位: {:0=2}:{:0=2}"_fmt, i + 1, m_data->record[i] / 60, m_data->record[i] % 60).draw(520, 300 + i * 60, Color(50));
		}
	}
};

struct Game : MyApp::Scene
{
	Sound m_sound = Sound(Wave(1s, [](double t) { return 0.6 * Sin(t * 1320 * TwoPi) * Exp(t * -10); }));
	Array<int32> m_row = { 0,1,2,3,4,5,6,7,8,9 };
	Array<int32> m_col = m_row;
	Grid<String> m_grid = Grid<String>(10, 10);
	int32 m_pos = 0;
	String m_input;
	Stopwatch m_stopwatch;

	void init() override
	{
		Shuffle(m_row);
		Shuffle(m_col);
	}

	void update() override
	{
		if (!m_stopwatch.isActive())
		{
			m_stopwatch.start();
		}

		const int32 posX = m_pos % 10, posY = m_pos / 10;

		Input::GetCharsHelper(m_input);

		if (m_input.length > 2)
		{
			m_input.resize(2);
		}

		Erase_if(m_input, [](wchar c) { return !IsDigit(c); });

		if ((m_grid[posY][posX] = m_input) == Format(m_col[posX] * m_row[posY]))
		{
			++m_pos;
			m_input.clear();
			m_sound.playMulti();
		}

		if (m_pos == 100)
		{
			m_stopwatch.pause();

			if ((m_data->previousRecord = m_stopwatch.s()) < m_data->record.back())
			{
				m_data->record.back() = m_data->previousRecord;
				std::sort(m_data->record.begin(), m_data->record.end());
				BinaryWriter(L"score.bin").write(m_data->record);
			}

			changeScene(L"Score", 2000);
		}
	}

	void draw() const override
	{
		const Vec2 offset(200, 30);

		if (m_pos < 100)
		{
			const double x = offset.x + (m_pos % 10 + 1) * 60, y = offset.y + (m_pos / 10 + 1) * 60;
			RectF(offset.x, y, 660, 60).draw(Color(50, 120, 200, 60));
			RectF(x, offset.y, 60, 660).draw(Color(50, 120, 200, 60));
			RectF(x, y, 60).draw();
		}

		for (int i : step(10))
		{
			FontAsset(L"Bold")(m_col[i]).drawCenter(Vec2(60 * (i + 1.5), 30) + offset, Color(50));
			FontAsset(L"Bold")(m_row[i]).drawCenter(Vec2(30, 60 * (i + 1.5)) + offset, Color(50));
		}

		for (int i : step(12))
		{
			Line(i * 60, 0, i * 60, 660).moveBy(offset).draw(1.75, Color(20));
			Line(0, i * 60, 660, i * 60).moveBy(offset).draw(1.75, Color(20));
		}

		for (auto p : step({ 10,10 }))
		{
			const Vec2 pos = offset.movedBy((p.x + 1.5) * 60, (p.y + 1.5) * 60) - FontAsset(L"Normal")(m_col[p.x] * m_row[p.y]).region().center;
			FontAsset(L"Normal")(m_grid[p]).draw(pos, Color(50));
		}

		Circle(1080, 160, 120).draw(Color(50, 120, 200, 160));
		const Vec2 pos = FontAsset(L"Big")(L"00:00").regionCenter(Vec2(1080, 160)).tl;
		FontAsset(L"Big")(L"{:0=2}:{:0=2}"_fmt, m_stopwatch.s() / 60, m_stopwatch.s() % 60).draw(pos);
	}
};

struct Score : MyApp::Scene
{
	Stopwatch m_stopwatch{ true };

	void update() override
	{
		if (Input::MouseL.clicked || Input::AnyKeyClicked())
		{
			changeScene(L"Title", 500);
		}
	}

	void draw() const override
	{
		bool newRecord = false;

		RectF(Min(m_stopwatch.ms(), Window::Width()), 90).setCenter(Window::Center().x, 200).draw(Color(50, 120, 200, 60));

		FontAsset(L"Big")(L"結果 {:0=2}:{:0=2}"_fmt, m_data->previousRecord / 60, m_data->previousRecord % 60).drawCenter(640, 200, Color(50));

		for (auto i : step(m_data->record.size()))
		{
			if (!newRecord && m_data->previousRecord == m_data->record[i])
			{
				Rect(Clamp(m_stopwatch.ms()- 200, 0, Window::Width()), 60).setCenter(Window::Center().x, 330 + i * 60)
					.draw(ColorF(1.0, 0.8, 0.5, 0.5 + 0.5 * Sin(System::FrameCount() / 20.0)));

				newRecord = true;
			}

			FontAsset(L"Bold")(L"{} 位: {:0=2}:{:0=2}"_fmt, i+1, m_data->record[i] / 60, m_data->record[i]%60).draw(520, 300 + i * 60, Color(50));
		}
	}
};

void Main()
{
	Window::SetTitle(L"百ます計算");
	Window::Resize(1280, 720);
	Graphics::SetBackground(Color(255, 244, 233));
	FontAsset::Register(L"Title", 56, Typeface::Heavy);
	FontAsset::Register(L"Big", 44, Typeface::Bold);
	FontAsset::Register(L"Bold", 28, Typeface::Bold);
	FontAsset::Register(L"Normal", 24, Typeface::Regular);

	MyApp manager;
	manager.setFadeColor(Palette::White);
	manager.add<Title>(L"Title");
	manager.add<Game>(L"Game");
	manager.add<Score>(L"Score");

	while (System::Update())
	{
		if (!manager.updateAndDraw())
			break;
	}
}
