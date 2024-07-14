#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include <map>
#include <fstream>
#include <numbers>
#include <string>

class Vec2 {
public:
	float x = 0, y = 0;
	Vec2() {}
	Vec2(float xin, float yin)
		: x(xin), y(yin) {}
	
	bool operator == (const Vec2& rhs) const{
		return (x == rhs.x && y == rhs.y);
	}
	bool operator != (const Vec2& rhs) const{
		return !(x == rhs.x && y == rhs.y);
	}
	Vec2 operator + (const Vec2& rhs) const{
		return Vec2(x + rhs.x, y + rhs.y);
	}
	Vec2 operator - (const Vec2& rhs) const{
		return Vec2(x - rhs.x, y - rhs.y);
	}
	Vec2 operator / (const float val) const{
		return Vec2(x / val, y / val);
	}
	Vec2 operator * (const float val) const{
		return Vec2(x * val, y * val);
	}

	void operator += (const Vec2& rhs) {
		x += rhs.x;
		y += rhs.y;
	}
	void operator -= (const Vec2& rhs) {
		x -= rhs.x;
		y -= rhs.y;
	}
	void operator *= (const float rhs) {
		x *= rhs;
		y *= rhs;
	}
	void operator /= (const float rhs) {
		x /= rhs;
		y /= rhs;
	}
	float distMag() const {
		return sqrt(x * x + y * y);
	}
	Vec2& normalize() {
		float d = (*this).distMag();
		x /= d;
		y /= d;
		return *this;
	}
};

class CTransform {
public:
	Vec2 pos      = { 0.0, 0.0 };
	Vec2 velocity = { 0.0, 0.0 };
	float angle   = 0;
	CTransform(const Vec2& p, const Vec2& v, float a)
		: pos(p), velocity(v), angle(a) {}
};

class CShape {
public:
	sf::CircleShape circle;
	CShape(float radius, int points, const sf::Color& fill, const sf::Color& outline, float thickness)
		:circle(radius, points){
		circle.setFillColor(fill);
		circle.setOutlineColor(outline);
		circle.setOutlineThickness(thickness);
		circle.setOrigin(radius, radius);
	}
};

class CCollision {
public:
	float radius = 0;
	CCollision(float r)
		: radius(r) {}
};

class CScore {
public:
	int score = 0;
	CScore(int s)
		: score(s) {}
};

class CLifespan {
public:
	int remaining = 0;
	int total = 0;
	CLifespan(int total)
		: remaining(total), total(total) {}
};

class CInput {
public:
	bool up = false;
	bool left = false;
	bool right = false;
	bool down = false;
	bool shootSpecial = false;
	CInput(){}
};

class Entity {
	bool        m_alive       = true;
	size_t      m_id          = 0;
	std::string m_tag         = "default";
	bool        m_smallEnemy = false;
	
public:
	Entity(const std::string& tag, const size_t id)
		:m_tag(tag), m_id(id) {}
	std::shared_ptr<CTransform> cTransform;
	std::shared_ptr<CShape>     cShape;
	std::shared_ptr<CCollision> cCollision;
	std::shared_ptr<CInput>     cInput;
	std::shared_ptr<CScore>     cScore;
	std::shared_ptr<CLifespan>  cLifespan;

	bool isActive() const {
		return m_alive;
	}
	bool isSmallEnenmy() const {
		return m_smallEnemy;
	}
	const std::string& tag() const {
		return m_tag;
	}
	const size_t id() const {
		return m_id;
	}
	void destroy() {
		m_alive = false;
	}
	void setSmallEnemy() {
		m_smallEnemy = true;
	}
};

typedef std::vector<std::shared_ptr<Entity>> EntityVec;
typedef std::map   <std::string, EntityVec>  EntityMap;

class EntityManager {
	EntityVec m_entities;
	EntityVec m_toAdd;
	EntityMap m_entityMap;
	size_t m_totalEntities = 0;
public:
	EntityManager() {}
	void update() {
		for (auto& e : m_toAdd) {
			m_entities.push_back(e);
			m_entityMap[e->tag()].push_back(e);
		}

		for (auto it = m_entities.begin(); it != m_entities.end(); ) {
			if (!((*it)->isActive())) {
				size_t idToKill = (*it)->id();
				for (auto& pair : m_entityMap) {
					if (pair.first == (*it)->tag()) {
						auto& vec = pair.second;
						for (auto vecIt = vec.begin(); vecIt != vec.end(); vecIt++) {
							if ((*vecIt)->id() == idToKill) {
								vecIt = vec.erase(vecIt);
								break;
							}
						}
						break;
					}
				}
				it = m_entities.erase(it);
			}
			else {
				it++;
			}
		}
		m_toAdd.clear();
	}
	std::shared_ptr<Entity> addEntity(const std::string& tag) {
		auto e = std::make_shared<Entity>(tag, m_totalEntities++);
		m_toAdd.push_back(e);
		return e;
	}
	EntityVec& getEntities() {
		return m_entities;
	}
	EntityVec& getEntities(const std::string& tag) {
		return m_entityMap[tag];
	}
};
// SR - shape radius, CR - collisition radius, S - speed, FR, FG, FB - fill color, OR, OG, OB - outline color, OT - outline thickness, V - shape vertices
//SMIN, SMAX - min/max speed, VMIN, VMAX - min/max vertices, L - small enemy lifespan, SP - spawn interval
//
struct PlayerConfig {
	int SR, CR; float S; int FR, FG, FB, OR, OG, OB, OT, V;
};

struct EnemyConfig {
	int SR, CR; float SMIN, SMAX; int OR, OG, OB, OT, VMIN, VMAX, L, SP;
};

struct BulletConfig {
	int SR, CR; float S; int FR, FG, FB, OR, OG, OB, OT, V, L;
};
struct TextConfig {
	std::string fontPath; int fontSize, fontR, fontG, fontB;
};
class Game {
	sf::RenderWindow m_window;
	EntityManager m_entities;
	sf::Font m_font;
	sf::Text m_text;
	std::string backgroundPath;
	sf::Texture backgroundTexture;
	sf::Sprite backgroundSprite;
	PlayerConfig m_playerConfig;
	EnemyConfig m_enemyConfig;
	BulletConfig m_bulletConfig;
	TextConfig m_textConfig;
	int m_score = 0;
	int m_currentFrame = 0;
	int m_lastEnemySpawnTime = 0;
	bool m_paused = false;
	bool m_running = true;
	int m_spacialWeaponRows = 8;
	int m_lastSpecialWeaponUseTime = -m_spacialWeaponRows - 1;
	int m_spacialWeaponCoolDown = 120;
	
	std::shared_ptr<Entity> m_player;

	void init(const std::string& path) {
		int wWidth;
		int wHeight;
		int frameLimit;

		std::ifstream fin(path);
		std::string temp;
		fin >> temp >> wWidth >> wHeight >> frameLimit >> temp >>
			temp >> m_textConfig.fontPath >> m_textConfig.fontSize >> m_textConfig.fontR >> m_textConfig.fontG >> m_textConfig.fontB >>
			temp >> m_playerConfig.SR >> m_playerConfig.CR >> m_playerConfig.S >> m_playerConfig.FR >> m_playerConfig.FG >> m_playerConfig.FB >> m_playerConfig.OR >> m_playerConfig.OG >> m_playerConfig.OB >> m_playerConfig.OT >> m_playerConfig.V >>
			temp >> m_enemyConfig.SR >> m_enemyConfig.CR >> m_enemyConfig.SMIN >> m_enemyConfig.SMAX >> m_enemyConfig.OR >> m_enemyConfig.OG >> m_enemyConfig.OB >> m_enemyConfig.OT >> m_enemyConfig.VMIN >> m_enemyConfig.VMAX >> m_enemyConfig.L >> m_enemyConfig.SP >>
			temp >> m_bulletConfig.SR >> m_bulletConfig.CR >> m_bulletConfig.S >> m_bulletConfig.FR >> m_bulletConfig.FG >> m_bulletConfig.FB >> m_bulletConfig.OR >> m_bulletConfig.OG >> m_bulletConfig.OB >> m_bulletConfig.OT >> m_bulletConfig.V >> m_bulletConfig.L >>
			temp >> backgroundPath;
		
		m_window.create(sf::VideoMode(wWidth, wHeight), "Shape wars");
		m_window.setFramerateLimit(frameLimit);

		score();
		background();
		spawnPlayer();
	}
	void setPause() {
		m_paused = !m_paused;
	}
	void sMovement() {
		float speed = sqrt((m_playerConfig.S * m_playerConfig.S) / 2);
		m_player->cTransform->velocity = { 0.0f, 0.0f };
		if (m_player->cInput->up) {
			if (m_player->cTransform->velocity.x) {
				m_player->cTransform->velocity.y = speed * -1;
			}
			else {
				m_player->cTransform->velocity.y = m_playerConfig.S * -1;
			}
		}
		if (m_player->cInput->down) {
			if (m_player->cTransform->velocity.x) {
				m_player->cTransform->velocity.y = speed * 1;
			}
			else {
				m_player->cTransform->velocity.y = m_playerConfig.S * 1;
			}
		}
		if (m_player->cInput->left) {
			if (m_player->cTransform->velocity.y) {
				m_player->cTransform->velocity.x = speed * -1;
			}
			else {
				m_player->cTransform->velocity.x = m_playerConfig.S * -1;
			}
		}
		if (m_player->cInput->right) {
			if (m_player->cTransform->velocity.y) {
				m_player->cTransform->velocity.x = speed * 1;
			}
			else {
				m_player->cTransform->velocity.x = m_playerConfig.S * 1;
			}
		}
		for (auto& e : m_entities.getEntities()) {
			e->cTransform->pos.x += e->cTransform->velocity.x;
			e->cTransform->pos.y += e->cTransform->velocity.y;
		}
		for (auto& e : m_entities.getEntities("enemy")) {
			if (e->cTransform->pos.x + e->cCollision->radius > m_window.getSize().x || e->cTransform->pos.x - e->cCollision->radius < 0) {
				e->cTransform->velocity.x *= -1;
			}
			if (e->cTransform->pos.y + e->cCollision->radius > m_window.getSize().y || e->cTransform->pos.y - e->cCollision->radius < 0) {
				e->cTransform->velocity.y *= -1;
			}
		}
		if (m_player->cTransform->pos.x + m_player->cCollision->radius > m_window.getSize().x) {
			m_player->cTransform->pos.x = m_window.getSize().x - m_player->cCollision->radius;
		}
		if (m_player->cTransform->pos.x - m_player->cCollision->radius < 0) {
			m_player->cTransform->pos.x = 0 + m_player->cCollision->radius;
		}
		if (m_player->cTransform->pos.y + m_player->cCollision->radius > m_window.getSize().y) {
			m_player->cTransform->pos.y = m_window.getSize().y - m_player->cCollision->radius;
		}
		if (m_player->cTransform->pos.y - m_player->cCollision->radius < 0) {
			m_player->cTransform->pos.y = 0 + m_player->cCollision->radius;
		}
	}
	void sLifespan() {
		for (auto& e : m_entities.getEntities()) {
			if (e->cLifespan && e->cShape) {
				if (e->cLifespan->remaining > 0) {
					e->cLifespan->remaining += -1;
					sf::Color fillColor = e->cShape->circle.getFillColor();
					sf::Color outlineColor = e->cShape->circle.getOutlineColor();
					float fade = ((float)e->cLifespan->remaining / (float)e->cLifespan->total);
					fillColor.a = static_cast<int>(round(255 * fade));
					outlineColor.a = static_cast<int>(round(255 * fade));
					e->cShape->circle.setFillColor(fillColor);
					e->cShape->circle.setOutlineColor(outlineColor);
				}
				else {
					e->destroy();
				}
			}
		}
	}
	void sRender() {
		m_window.clear();
		m_window.draw(backgroundSprite);
		for (auto& e : m_entities.getEntities()) {
			e->cShape->circle.setPosition(e->cTransform->pos.x, e->cTransform->pos.y);
			e->cTransform->angle += 1.0f;
			e->cShape->circle.setRotation(e->cTransform->angle);
			m_window.draw(e->cShape->circle);
		}
		m_text.setString("Score: " + std::to_string(m_score));
		m_window.draw(m_text);
		m_window.display();
	}
	void sEnemySpawner() {
		if (m_currentFrame - m_lastEnemySpawnTime > m_enemyConfig.SP) {
			spawnEnemy();
		}
	}
	void sCollision() {
		for (auto& e : m_entities.getEntities("enemy")) {
			for (auto& b : m_entities.getEntities("bullet")) {
				Vec2 e_b_dist = e->cTransform->pos - b->cTransform->pos;
				if ((e_b_dist.x * e_b_dist.x + e_b_dist.y * e_b_dist.y) < (e->cCollision->radius + b->cCollision->radius) * (e->cCollision->radius + b->cCollision->radius)) {
					e->destroy();
					b->destroy();
					spawnSmallEnemies(e);
					if (e->cScore->score) {
						m_score += e->cScore->score;
					}
				}
			}
			Vec2 e_p_dist = e->cTransform->pos - m_player->cTransform->pos;
			if ((e_p_dist.x * e_p_dist.x + e_p_dist.y * e_p_dist.y) < (e->cCollision->radius + m_player->cCollision->radius) * (e->cCollision->radius + m_player->cCollision->radius)) {
				e->destroy();
				spawnSmallEnemies(e);
				m_score = 0;
				m_player->destroy();
				spawnPlayer();
			}
		}

	}

	void spawnPlayer() {
		auto entity = m_entities.addEntity("player");

		entity->cTransform = std::make_shared<CTransform>(Vec2(m_window.getSize().x / 2.0f, m_window.getSize().y / 2.0f), Vec2(0.0f, 0.0f), 0.0f);
		entity->cShape = std::make_shared<CShape>(m_playerConfig.SR, m_playerConfig.V, sf::Color(m_playerConfig.FR, m_playerConfig.FG, m_playerConfig.FB), sf::Color(m_playerConfig.OR, m_playerConfig.OG, m_playerConfig.OB), m_playerConfig.OT);
		entity->cCollision = std::make_shared<CCollision>(m_playerConfig.CR);
		entity->cInput = std::make_shared<CInput>();
		m_player = entity;
	}
	void spawnEnemy() {
		float p_pos_x = m_player->cTransform->pos.x;
		float p_pos_y = m_player->cTransform->pos.y;
		float p_rad = m_player->cCollision->radius;
		auto entity = m_entities.addEntity("enemy");
		Vec2 e_pos;
		do{
			float ex = static_cast<float>(rand() % static_cast<int>(round(m_window.getSize().x - 2 * m_enemyConfig.SR + 1)) + m_enemyConfig.SR);
			float ey = static_cast<float>(rand() % static_cast<int>(round(m_window.getSize().y - 2 * m_enemyConfig.SR + 1)) + m_enemyConfig.SR);
			e_pos = { ex, ey };
		}while (e_pos.x > p_pos_x - p_rad * 10 && p_pos_x + p_rad * 10 > e_pos.x && e_pos.y > p_pos_y - p_rad * 10 && p_pos_y + p_rad * 10 > e_pos.y);
		int vertices = rand() % (m_enemyConfig.VMAX - m_enemyConfig.VMIN + 1) + m_enemyConfig.VMIN;
		int fillR = rand() % 255 + 1;
		int fillG = rand() % 255 + 1;
		int fillB = rand() % 255 + 1;
		Vec2 target = { static_cast<float>(rand() % m_window.getSize().x), static_cast<float>(rand() % m_window.getSize().y) };
		float speed;
		if (m_enemyConfig.SMAX - m_enemyConfig.SMIN != 0) {
			speed = static_cast<float>(rand() % (static_cast<int>(m_enemyConfig.SMAX - m_enemyConfig.SMIN + 1)) + m_enemyConfig.SMIN);
		}
		else {
			speed = m_enemyConfig.SMAX;
		}
		Vec2 velos = (target - e_pos).normalize() * speed;
		entity->cTransform = std::make_shared<CTransform>(Vec2(e_pos), Vec2(velos), 0.0f);
		entity->cShape = std::make_shared<CShape>(m_enemyConfig.SR, vertices, sf::Color(fillR, fillG, fillB), sf::Color(m_enemyConfig.OR, m_enemyConfig.OG, m_enemyConfig.OB), m_enemyConfig.OT);
		entity->cCollision = std::make_shared<CCollision>(m_enemyConfig.CR);
		entity->cScore = std::make_shared<CScore>(100);
		m_lastEnemySpawnTime = m_currentFrame;
	}
	void spawnSmallEnemies(const std::shared_ptr<Entity>& entity) {
		if (!entity->isSmallEnenmy()) {
			size_t points = entity->cShape->circle.getPointCount();
			float speed = entity->cTransform->velocity.distMag();
			float a = static_cast<float>((std::fmod(entity->cTransform->angle, 360) * std::numbers::pi) / 180);
			float increment = static_cast<float>((std::numbers::pi * 2) / points);
			for (size_t i = 0; i < points; i++) {
				float current_a = a + increment * i;
				Vec2 vel = { sin(current_a) * speed, -cos(current_a) * speed };
				auto smallEntity = m_entities.addEntity("enemy");
				smallEntity->setSmallEnemy();
				smallEntity->cTransform = std::make_shared<CTransform>(Vec2(entity->cTransform->pos), Vec2(vel), 0.0f);
				smallEntity->cShape = std::make_shared<CShape>(entity->cShape->circle.getRadius() / 2, points, entity->cShape->circle.getFillColor(), entity->cShape->circle.getOutlineColor(), entity->cShape->circle.getOutlineThickness() / 2);
				smallEntity->cCollision = std::make_shared<CCollision>(entity->cCollision->radius / 2);
				smallEntity->cLifespan = std::make_shared<CLifespan>(m_enemyConfig.L);
				smallEntity->cScore = std::make_shared<CScore>(entity->cScore->score * 2);
			}
		}
	}
	void spawnBullet(const std::shared_ptr<Entity>& entity, const Vec2& target) {
		Vec2 normalVec = (target - entity->cTransform->pos).normalize();
		auto bullet = m_entities.addEntity("bullet");
		bullet->cTransform = std::make_shared<CTransform>(Vec2(entity->cTransform->pos), Vec2(normalVec.x * m_bulletConfig.S, normalVec.y * m_bulletConfig.S), 0.0f);
		bullet->cShape = std::make_shared<CShape>(m_bulletConfig.SR, m_bulletConfig.V, sf::Color(m_bulletConfig.FR, m_bulletConfig.FG, m_bulletConfig.FB), sf::Color(m_bulletConfig.OR, m_bulletConfig.OG, m_bulletConfig.OB), m_bulletConfig.OT);
		bullet->cCollision = std::make_shared<CCollision>(m_bulletConfig.CR);
		bullet->cLifespan = std::make_shared<CLifespan>(m_bulletConfig.L);
	}
	void spawnSpecialWeapon() {
		if (m_player->cInput->shootSpecial || m_currentFrame - m_lastSpecialWeaponUseTime < m_spacialWeaponRows) {
			float numOfBull = 36.0f;
			float incriment = 2 * std::numbers::pi / numOfBull;
			float a = std::fmod(m_player->cTransform->angle, 360) * 2 * std::numbers::pi / 180;
			for (int i = 0; i < numOfBull; i++) {
				float x = cos(i * incriment + a);
				float y = sin(i * incriment + a);
				Vec2 targ = {x, y};
				spawnBullet(m_player, targ + m_player->cTransform->pos);
			}
			m_player->cInput->shootSpecial = false;
		}
	}
	void sUserInput() {
		sf::Event event;
		while (m_window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				m_running = false;
			}
			if (event.type == sf::Event::KeyPressed) {
				switch (event.key.code) {
				case sf::Keyboard::W:
					m_player->cInput->up = true;
					break;
				case sf::Keyboard::S:
					m_player->cInput->down = true;
					break;
				case sf::Keyboard::A:
					m_player->cInput->left = true;
					break;
				case sf::Keyboard::D:
					m_player->cInput->right = true;
					break;
				case sf::Keyboard::P:
					setPause();
					break;
				}
			}
			if (event.type == sf::Event::KeyReleased) {
				switch (event.key.code) {
				case sf::Keyboard::W:
					m_player->cInput->up = false;
					break;
				case sf::Keyboard::S:
					m_player->cInput->down = false;
					break;
				case sf::Keyboard::A:
					m_player->cInput->left = false;
					break;
				case sf::Keyboard::D:
					m_player->cInput->right = false;
					break;
				}
			}
			if (event.type == sf::Event::MouseButtonPressed) {
				if (event.mouseButton.button == sf::Mouse::Left) {
					spawnBullet(m_player, Vec2((float)event.mouseButton.x, (float)event.mouseButton.y));
				}
				if (event.mouseButton.button == sf::Mouse::Right) {
					if (m_currentFrame > m_lastSpecialWeaponUseTime + m_spacialWeaponCoolDown) {
						m_player->cInput->shootSpecial = true;
						m_lastSpecialWeaponUseTime = m_currentFrame;
					}
				}
			}
		}
	}
	void score() {
		m_font.loadFromFile(m_textConfig.fontPath);
		m_text.setFont(m_font);
		m_text.setCharacterSize(m_textConfig.fontSize);
		m_text.setFillColor(sf::Color(m_textConfig.fontR, m_textConfig.fontG, m_textConfig.fontB));
		m_text.setPosition(20, 20);
	}
	void background() {
		backgroundTexture.loadFromFile(backgroundPath);
		backgroundSprite.setTexture(backgroundTexture);
		backgroundSprite.setScale(0.3f, 0.3f);
	}
public:
	Game(const std::string& config) {
		init(config);
	}
	void run() {
		while (m_running) {
			m_entities.update();
			if (!m_paused) {
				sEnemySpawner();
				sMovement();
				sCollision();
				sLifespan();
				spawnSpecialWeapon();
			}
			sUserInput();
			sRender();
			m_currentFrame++;
		}
	}
};
