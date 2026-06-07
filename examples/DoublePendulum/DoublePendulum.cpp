#include <Translucence.hpp>
#include <vector>
#include <cmath>

struct PendulumState {
    std::vector<float> a;    // Angles
    std::vector<float> av;   // Angular velocities
    std::vector<float> l;    // Lengths
    std::vector<float> m;    // Masses
    std::vector<Circle> joints;
    std::vector<Line> rods;
    int n = 2;

    void reset(int numJoints, float2 startPos) {
        n = numJoints;
        a.assign(n, 1.57f);
        av.assign(n, 0.0f);
        l.assign(n, 400.0f / n);
        m.assign(n, 10.0f);
        joints.resize(n + 1);
        rods.resize(n);
        
        joints[0] = {startPos, 10};
        updatePositions();
    }

    void updatePositions() {
        for (int i = 0; i < n; ++i) {
            joints[i + 1].pos = {
                joints[i].pos.x + l[i] * sin(a[i]),
                joints[i].pos.y + l[i] * cos(a[i])
            };
            rods[i] = {joints[i].pos, joints[i + 1].pos, 5};
            joints[i + 1].radius = 10;
        }
    }
};

// Simple Gaussian elimination to solve M * x = f
std::vector<float> solveLinearSystem(std::vector<std::vector<float>>& M, std::vector<float>& f) {
    int n = f.size();
    for (int i = 0; i < n; i++) {
        // Pivot
        int pivot = i;
        for (int j = i + 1; j < n; j++) {
            if (std::abs(M[j][i]) > std::abs(M[pivot][i])) pivot = j;
        }
        std::swap(M[i], M[pivot]);
        std::swap(f[i], f[pivot]);

        for (int j = i + 1; j < n; j++) {
            float factor = M[j][i] / M[i][i];
            f[j] -= factor * f[i];
            for (int k = i; k < n; k++) {
                M[j][k] -= factor * M[i][k];
            }
        }
    }

    std::vector<float> x(n);
    for (int i = n - 1; i >= 0; i--) {
        float sum = 0;
        for (int j = i + 1; j < n; j++) {
            sum += M[i][j] * x[j];
        }
        x[i] = (f[i] - sum) / M[i][i];
    }
    return x;
}
std::pair<std::vector<float>, std::vector<float>> get_derivatives(int n, const std::vector<float>& a, const std::vector<float>& av, const std::vector<float>& l, const std::vector<float>& m, float g) {
    std::vector<std::vector<float>> M(n, std::vector<float>(n));
    std::vector<float> f(n);

    for (int i = 0; i < n; i++) {
        // Mass matrix M_ij
        for (int j = 0; j < n; j++) {
            float mass_sum = 0;
            for (int k = std::max(i, j); k < n; k++) mass_sum += m[k];
            M[i][j] = mass_sum * l[i] * l[j] * std::cos(a[i] - a[j]);
        }

        // Force vector f_i
        float term1 = 0;
        for (int j = 0; j < n; j++) {
            float mass_sum = 0;
            for (int k = std::max(i, j); k < n; k++) mass_sum += m[k];
            term1 -= mass_sum * l[i] * l[j] * av[j] * av[j] * std::sin(a[i] - a[j]);
        }

        float mass_sum_i = 0;
        for (int k = i; k < n; k++) mass_sum_i += m[k];
        float term2 = -mass_sum_i * g * l[i] * std::sin(a[i]);

        f[i] = term1 + term2;
    }

    return {av, solveLinearSystem(M, f)};
}

int main() {
    Application app;
    app.create(1000, 800, "Translucence Pendulum Demo");
    Renderer renderer(app);
    EventSystem events(app);

    PendulumState state;
    state.reset(2, {500, 150});

    int dragIdx = -1;
    int speed = 2000;
    bool playing = false;
    float g = 9.81f;
    float damping = 1.0f;
    float accumulator = 0.0f;
    const float physicsDt = 0.001f;
    Tail tail;
    tail.maxLength = 20000;


    while (app.isRunning()) {
        app.update();
        float dt = app.getDeltaTime();
        if (dt > 0.25f) dt = 0.25f;
        
        accumulator += dt * (speed / 100.0f);
        events.runEvents();
        renderer.clearBackground(Color::BgDeep);

        if (Input::isKeyDown(Input::Key::ESCAPE)) app.setRunning(false);

        // Number keys for toggling pendulum count
        if (Input::isKeyPressed(Input::Key::NUM_2)) { state.reset(2, state.joints[0].pos); tail.points.clear(); playing = false; }
        if (Input::isKeyPressed(Input::Key::NUM_3)) { state.reset(3, state.joints[0].pos); tail.points.clear(); playing = false; }
        if (Input::isKeyPressed(Input::Key::NUM_4)) { state.reset(4, state.joints[0].pos); tail.points.clear(); playing = false; }
        if (Input::isKeyPressed(Input::Key::NUM_5)) { state.reset(5, state.joints[0].pos); tail.points.clear(); playing = false; }
        if (Input::isKeyPressed(Input::Key::NUM_6)) { state.reset(6, state.joints[0].pos); tail.points.clear(); playing = false; }
        if (Input::isKeyPressed(Input::Key::NUM_7)) { state.reset(7, state.joints[0].pos); tail.points.clear(); playing = false; }
        if (Input::isKeyPressed(Input::Key::NUM_8)) { state.reset(8, state.joints[0].pos); tail.points.clear(); playing = false; }
        if (Input::isKeyPressed(Input::Key::NUM_9)) { state.reset(9, state.joints[0].pos); tail.points.clear(); playing = false; }

        if (Input::isMouseClicked()) {
            for (int i = 0; i <= state.n; ++i) {
                if (Input::isMouseHoveringCircle(Input::getMousePos(), state.joints[i])) {
                    dragIdx = i;
                    break;
                }
            }
        } else {
            dragIdx = -1;
        }

        if (dragIdx == 0) {
            state.joints[0].pos = Input::getMousePos();
        } else if (dragIdx > 0) {
            float2 mPos = Input::getMousePos();
            state.a[dragIdx - 1] = atan2(mPos.x - state.joints[dragIdx - 1].pos.x, mPos.y - state.joints[dragIdx - 1].pos.y);
        }

        if (Input::isKeyPressed(Input::Key::R)) {
            state.reset(state.n, {500, 150});
            tail.points.clear();
        }
        if (Input::isKeyPressed(Input::Key::SPACE)) {
            playing = !playing;
        }

        float scroll = Input::getMouseWheelY();
        if (scroll != 0) {
            float change = scroll * 5.0f;
            for (int i = 0; i < state.n; i++) {
                state.l[i] = std::max(10.0f, state.l[i] + change);
            }
            tail.points.clear();
        }

        if (playing && dragIdx == -1) {
            while (accumulator >= physicsDt) {
                // RK4 Integration
                auto k1 = get_derivatives(state.n, state.a, state.av, state.l, state.m, g);
                
                std::vector<float> a2(state.n), av2(state.n);
                for(int i=0; i<state.n; ++i) {
                    a2[i] = state.a[i] + k1.first[i] * physicsDt * 0.5f;
                    av2[i] = state.av[i] + k1.second[i] * physicsDt * 0.5f;
                }
                auto k2 = get_derivatives(state.n, a2, av2, state.l, state.m, g);

                std::vector<float> a3(state.n), av3(state.n);
                for(int i=0; i<state.n; ++i) {
                    a3[i] = state.a[i] + k2.first[i] * physicsDt * 0.5f;
                    av3[i] = state.av[i] + k2.second[i] * physicsDt * 0.5f;
                }
                auto k3 = get_derivatives(state.n, a3, av3, state.l, state.m, g);

                std::vector<float> a4(state.n), av4(state.n);
                for(int i=0; i<state.n; ++i) {
                    a4[i] = state.a[i] + k3.first[i] * physicsDt;
                    av4[i] = state.av[i] + k3.second[i] * physicsDt;
                }
                auto k4 = get_derivatives(state.n, a4, av4, state.l, state.m, g);

                for (int i = 0; i < state.n; i++) {
                    state.a[i] += (physicsDt / 6.0f) * (k1.first[i] + 2 * k2.first[i] + 2 * k3.first[i] + k4.first[i]);
                    state.av[i] += (physicsDt / 6.0f) * (k1.second[i] + 2 * k2.second[i] + 2 * k3.second[i] + k4.second[i]);
                    state.av[i] *= damping;
                }

                accumulator -= physicsDt;

                state.updatePositions();
                float2 tip = state.joints.back().pos;
                if (tail.points.empty() || Math::distance(tail.points.back(), tip) > 2.0f) {
                    tail.points.push_back(tip);
                    if (tail.points.size() > 1000) tail.points.erase(tail.points.begin());
                }
            }
        } else {
            if (!playing) state.av.assign(state.n, 0.0f);
            accumulator = 0;
        }
        state.updatePositions();

        for (auto& rod : state.rods) renderer.drawLine(rod, Color::Silver);
        for (auto& joint : state.joints) renderer.drawCircle(joint, Color::White);

        renderer.drawTail(tail, state.joints.back().pos, Color::SlateGray, 5);
        renderer.drawText(playing ? "Playing" : "Paused", {50, 50}, Color::White, 20);
        renderer.drawText("Pendulums: " + std::to_string(state.n), {50, 80}, Color::White, 20);

        float listSpacing = 25.0f;
        float listHeight = state.n * listSpacing + 30.0f;
        float listY = (float)app.getHeight() - listHeight;

        renderer.drawText("Angles:", {50, listY}, Color::White, 20);
        renderer.drawList(toStringVec(state.a), {50, listY + 30.0f}, Color::Silver, 20);

        renderer.drawText("Angular velocities:", {300, listY}, Color::White, 20);
        renderer.drawList(toStringVec(state.av), {300, listY + 30.0f}, Color::Silver, 20);
        renderer.render();
    }
    return 0;
}