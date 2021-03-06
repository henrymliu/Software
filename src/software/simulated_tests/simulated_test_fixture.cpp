#include "software/simulated_tests/simulated_test_fixture.h"

#include "software/util/logger/init.h"
#include "software/util/time/duration.h"

void SimulatedTest::SetUp()
{
    Util::Logger::LoggerSingleton::initializeLogger();
    backend = std::make_shared<SimulatorBackend>(
        Duration::fromMilliseconds(5), Duration::fromSeconds(1.0 / 30.0),
        SimulatorBackend::SimulationSpeed::FAST_SIMULATION);
    world_state_validator = std::make_shared<WorldStateValidator>();
    ai_wrapper            = std::make_shared<AIWrapper>();

    // The world_state_observer observes the World from the backend, and then the ai
    // observes the World from the WorldStateObserver. Because we know the AI will not
    // run until it gets a new World, and the SimulatorBackend will not pubish another
    // world until it has received primitives, we can guarantee that each step in the
    // test pipeline will complete before the next. The steps are:
    // 1. Simulate and publish new world
    // 2. Validate World
    // 3. AI makes decisions based on new world
    // Overall this makes the tests deterministic because one step will not asynchronously
    // run way faster than another and lose data.
    backend->Subject<World>::registerObserver(world_state_validator);
    world_state_validator->Subject<World>::registerObserver(ai_wrapper);
    ai_wrapper->Subject<ConstPrimitiveVectorPtr>::registerObserver(backend);
}

void SimulatedTest::TearDown()
{
    backend->stopSimulation();
}

void SimulatedTest::enableVisualizer()
{
    // The XDG_RUNTIME_DIR environment variable must be set in order for the
    // Visualizer to work properly. If it's not set, the Visualizer initialization
    // will fail with an error like "qt.qpa.screen: QXcbConnection: Could not
    // connect to display" In order for this environment variable to be set
    // correctly these test targets MUST be run with 'bazel run' rather than
    // 'bazel test'
    auto xdg_runtime_dir = std::getenv("XDG_RUNTIME_DIR");
    if (!xdg_runtime_dir)
    {
        LOG(WARNING)
            << "The XDG_RUNTIME_DIR environment variable was not set. It must"
               " be set in order for the Visualizer to run properly. If you want"
               "to enable the Visualizer for these tests, make sure you run the"
               "targets with 'bazel run' rather than 'bazel test' so the environment"
               "variables are set properly.";
        ADD_FAILURE() << "Cannot enable Visualizer" << std::endl;
    }

    // We mock empty argc and argv since we don't have access to them when running
    // tests These arguments do not matter for simply running the Visualizer
    char *argv[] = {NULL};
    int argc     = sizeof(argv) / sizeof(char *) - 1;
    visualizer   = std::make_shared<VisualizerWrapper>(argc, argv);
    backend->Subject<World>::registerObserver(visualizer);
    backend->Subject<RobotStatus>::registerObserver(visualizer);

    // Simulate in realtime if we are using the Visualizer so we can actually see
    // things at a reasonably realistic speed
    backend->setSimulationSpeed(SimulatorBackend::SimulationSpeed::REALTIME_SIMULATION);
}
