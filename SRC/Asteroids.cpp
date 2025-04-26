#include "Asteroid.h"
#include "Asteroids.h"
#include "Animation.h"
#include "AnimationManager.h"
#include "GameUtil.h"
#include "GameWindow.h"
#include "GameWorld.h"
#include "GameDisplay.h"
#include "Spaceship.h"
#include "BoundingShape.h"
#include "BoundingSphere.h"
#include "GUILabel.h"
#include "Explosion.h"
#include "GUIContainer.h"

Asteroids::Asteroids(int argc, char* argv[])
    : GameSession(argc, argv), mIsStartScreen(true)
{
    mLevel = 0;
    mAsteroidCount = 0;
}

Asteroids::~Asteroids(void)
{
}

void Asteroids::Start()
{
    shared_ptr<Asteroids> thisPtr = shared_ptr<Asteroids>(this);
    mGameWorld->AddListener(thisPtr.get());
    mGameWindow->AddKeyboardListener(thisPtr);
    mGameWorld->AddListener(&mScoreKeeper);
    mScoreKeeper.AddListener(thisPtr);

    GLfloat ambient_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat diffuse_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient_light);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse_light);
    glEnable(GL_LIGHT0);

    Animation* explosion_anim = AnimationManager::GetInstance().CreateAnimationFromFile("explosion", 64, 1024, 64, 64, "explosion_fs.png");
    Animation* asteroid1_anim = AnimationManager::GetInstance().CreateAnimationFromFile("asteroid1", 128, 8192, 128, 128, "asteroid1_fs.png");
    Animation* spaceship_anim = AnimationManager::GetInstance().CreateAnimationFromFile("spaceship", 128, 128, 128, 128, "spaceship_fs.png");

    if (!mIsStartScreen)
    {
        mGameWorld->AddObject(CreateSpaceship());
    }

    CreateAsteroids(10);
    CreateGUI();

    // Set visibility for start screen
    if (mIsStartScreen)
    {
        mScoreLabel->SetVisible(false);
        mLivesLabel->SetVisible(false);
        mGameOverLabel->SetVisible(false);
        mStartGameLabel->SetVisible(true);
        mDifficultyLabel->SetVisible(true);
        mInstructionsLabel->SetVisible(true);
        mHighScoresLabel->SetVisible(true);
    }
    else
    {
        mStartGameLabel->SetVisible(false);
        mDifficultyLabel->SetVisible(false);
        mInstructionsLabel->SetVisible(false);
        mHighScoresLabel->SetVisible(false);
    }

    mGameWorld->AddListener(&mPlayer);
    mPlayer.AddListener(thisPtr);

    GameSession::Start();
}

void Asteroids::Stop()
{
    GameSession::Stop();
}

void Asteroids::OnKeyPressed(uchar key, int x, int y)
{
    switch (key)
    {
    case ' ':
        mSpaceship->Shoot();
        break;
    default:
        break;
    }
}

void Asteroids::OnKeyReleased(uchar key, int x, int y) {}

void Asteroids::OnSpecialKeyPressed(int key, int x, int y)
{
    switch (key)
    {
    case GLUT_KEY_UP: mSpaceship->Thrust(10); break;
    case GLUT_KEY_LEFT: mSpaceship->Rotate(90); break;
    case GLUT_KEY_RIGHT: mSpaceship->Rotate(-90); break;
    default: break;
    }
}

void Asteroids::OnSpecialKeyReleased(int key, int x, int y)
{
    switch (key)
    {
    case GLUT_KEY_UP: mSpaceship->Thrust(0); break;
    case GLUT_KEY_LEFT: mSpaceship->Rotate(0); break;
    case GLUT_KEY_RIGHT: mSpaceship->Rotate(0); break;
    default: break;
    }
}

void Asteroids::OnObjectRemoved(GameWorld* world, shared_ptr<GameObject> object)
{
    if (object->GetType() == GameObjectType("Asteroid"))
    {
        shared_ptr<GameObject> explosion = CreateExplosion();
        explosion->SetPosition(object->GetPosition());
        explosion->SetRotation(object->GetRotation());
        mGameWorld->AddObject(explosion);

        mAsteroidCount--;
        if (mAsteroidCount <= 0)
        {
            SetTimer(500, START_NEXT_LEVEL);
        }
    }
}

void Asteroids::OnTimer(int value)
{
    if (value == CREATE_NEW_PLAYER)
    {
        mSpaceship->Reset();
        mGameWorld->AddObject(mSpaceship);
    }

    if (value == START_NEXT_LEVEL)
    {
        mLevel++;
        int num_asteroids = 10 + 2 * mLevel;
        CreateAsteroids(num_asteroids);
    }

    if (value == SHOW_GAME_OVER)
    {
        mGameOverLabel->SetVisible(true);
    }
}

shared_ptr<GameObject> Asteroids::CreateSpaceship()
{
    mSpaceship = make_shared<Spaceship>();
    mSpaceship->SetBoundingShape(make_shared<BoundingSphere>(mSpaceship->GetThisPtr(), 4.0f));
    shared_ptr<Shape> bullet_shape = make_shared<Shape>("bullet.shape");
    mSpaceship->SetBulletShape(bullet_shape);
    Animation* anim_ptr = AnimationManager::GetInstance().GetAnimationByName("spaceship");
    shared_ptr<Sprite> spaceship_sprite =
        make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
    mSpaceship->SetSprite(spaceship_sprite);
    mSpaceship->SetScale(0.1f);
    mSpaceship->Reset();
    return mSpaceship;
}

void Asteroids::CreateAsteroids(const uint num_asteroids)
{
    mAsteroidCount = num_asteroids;

    for (uint i = 0; i < num_asteroids; i++)
    {
        Animation* anim_ptr = AnimationManager::GetInstance().GetAnimationByName("asteroid1");
        shared_ptr<Sprite> asteroid_sprite
            = make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
        asteroid_sprite->SetLoopAnimation(true);
        shared_ptr<GameObject> asteroid = make_shared<Asteroid>();
        asteroid->SetBoundingShape(make_shared<BoundingSphere>(asteroid->GetThisPtr(), 10.0f));
        asteroid->SetSprite(asteroid_sprite);
        asteroid->SetScale(0.2f);
        mGameWorld->AddObject(asteroid);
    }
}

void Asteroids::CreateGUI()
{
    mGameDisplay->GetContainer()->SetBorder(GLVector2i(10, 10));

    // Create menu labels directly in game display
    mStartGameLabel = make_shared<GUILabel>("Start Game");
    mStartGameLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
    mStartGameLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
    mStartGameLabel->SetVisible(mIsStartScreen);
    shared_ptr<GUIComponent> start_game_component = static_pointer_cast<GUIComponent>(mStartGameLabel);
    mGameDisplay->GetContainer()->AddComponent(start_game_component, GLVector2f(0.5f, 0.65f));

    mDifficultyLabel = make_shared<GUILabel>("Difficulty");
    mDifficultyLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
    mDifficultyLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
    mDifficultyLabel->SetVisible(mIsStartScreen);
    shared_ptr<GUIComponent> difficulty_component = static_pointer_cast<GUIComponent>(mDifficultyLabel);
    mGameDisplay->GetContainer()->AddComponent(difficulty_component, GLVector2f(0.5f, 0.55f));

    mInstructionsLabel = make_shared<GUILabel>("Instructions");
    mInstructionsLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
    mInstructionsLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
    mInstructionsLabel->SetVisible(mIsStartScreen);
    shared_ptr<GUIComponent> instructions_component = static_pointer_cast<GUIComponent>(mInstructionsLabel);
    mGameDisplay->GetContainer()->AddComponent(instructions_component, GLVector2f(0.5f, 0.45f));

    mHighScoresLabel = make_shared<GUILabel>("High Scores");
    mHighScoresLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
    mHighScoresLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
    mHighScoresLabel->SetVisible(mIsStartScreen);
    shared_ptr<GUIComponent> high_scores_component = static_pointer_cast<GUIComponent>(mHighScoresLabel);
    mGameDisplay->GetContainer()->AddComponent(high_scores_component, GLVector2f(0.5f, 0.35f));

    // Create other GUI elements
    mScoreLabel = make_shared<GUILabel>("Score: 0");
    mScoreLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_TOP);
    shared_ptr<GUIComponent> score_component = static_pointer_cast<GUIComponent>(mScoreLabel);
    mGameDisplay->GetContainer()->AddComponent(score_component, GLVector2f(0.0f, 1.0f));

    mLivesLabel = make_shared<GUILabel>("Lives: 3");
    mLivesLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_BOTTOM);
    shared_ptr<GUIComponent> lives_component = static_pointer_cast<GUIComponent>(mLivesLabel);
    mGameDisplay->GetContainer()->AddComponent(lives_component, GLVector2f(0.0f, 0.0f));

    mGameOverLabel = shared_ptr<GUILabel>(new GUILabel("GAME OVER"));
    mGameOverLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
    mGameOverLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
    mGameOverLabel->SetVisible(false);
    shared_ptr<GUIComponent> game_over_component = static_pointer_cast<GUIComponent>(mGameOverLabel);
    mGameDisplay->GetContainer()->AddComponent(game_over_component, GLVector2f(0.5f, 0.5f));
}

void Asteroids::OnScoreChanged(int score)
{
    std::ostringstream msg_stream;
    msg_stream << "Score: " << score;
    std::string score_msg = msg_stream.str();
    mScoreLabel->SetText(score_msg);
}

void Asteroids::OnPlayerKilled(int lives_left)
{
    shared_ptr<GameObject> explosion = CreateExplosion();
    explosion->SetPosition(mSpaceship->GetPosition());
    explosion->SetRotation(mSpaceship->GetRotation());
    mGameWorld->AddObject(explosion);

    std::ostringstream msg_stream;
    msg_stream << "Lives: " << lives_left;
    std::string lives_msg = msg_stream.str();
    mLivesLabel->SetText(lives_msg);

    if (lives_left > 0)
    {
        SetTimer(1000, CREATE_NEW_PLAYER);
    }
    else
    {
        SetTimer(500, SHOW_GAME_OVER);
    }
}

shared_ptr<GameObject> Asteroids::CreateExplosion()
{
    Animation* anim_ptr = AnimationManager::GetInstance().GetAnimationByName("explosion");
    shared_ptr<Sprite> explosion_sprite =
        make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
    explosion_sprite->SetLoopAnimation(false);
    shared_ptr<GameObject> explosion = make_shared<Explosion>();
    explosion->SetSprite(explosion_sprite);
    explosion->Reset();
    return explosion;
}