#include <focusrite/e2e/ComponentSearch.h>

namespace focusrite::e2e
{
std::vector<juce::Component *>
getDirectDescendantsMatching (juce::Component & parent,
                              const std::function<bool (juce::Component &)> & predicate)
{
    auto children = parent.getChildren ();
    children.removeIf ([&] (auto && child) { return child == nullptr || ! predicate (*child); });
    return {children.begin (), children.end ()};
}

juce::Component * matchChildComponent (juce::Component & component,
                                       const std::function<bool (juce::Component &)> & predicate,
                                       int & skip)
{
    jassert (skip >= 0);

    auto matchingChildren = getDirectDescendantsMatching (component, predicate);

    if (skip < int (matchingChildren.size ()))
        return matchingChildren [skip];

    skip -= int (matchingChildren.size ());

    for (auto * child : component.getChildren ())
    {
        if (! child)
            continue;

        if (auto * foundComponent = matchChildComponent (*child, predicate, skip))
            return foundComponent;
    }

    return nullptr;
}

juce::Component * findChildComponent (juce::Component & component,
                                      const std::function<bool (juce::Component &)> & predicate,
                                      int skip = 0)
{
    return matchChildComponent (component, predicate, skip);
}

std::vector<juce::TopLevelWindow *> getTopLevelWindows ()
{
    std::vector<juce::TopLevelWindow *> windows;

    for (int windowIndex = 0; windowIndex < juce::TopLevelWindow::getNumTopLevelWindows ();
         ++windowIndex)
        if (auto window = juce::TopLevelWindow::getTopLevelWindow (windowIndex))
            windows.push_back (window);

    return windows;
}

juce::Component * findComponent (const std::function<bool (juce::Component &)> & predicate,
                                 int skip = 0)
{
    for (auto & window : getTopLevelWindows ())
        if (auto component = findChildComponent (*window, predicate, skip))
            return component;

    return nullptr;
}

std::function<bool (juce::Component &)> createComponentMatcher (const juce::String & componentId)
{
    return [componentId] (auto && component) -> bool
    {
        return component.getProperties ()
                   .getWithDefault (ComponentSearch::testId, {})
                   .toString ()
                   .matchesWildcard (componentId, false) &&
               component.isVisible () && component.isShowing ();
    };
}

juce::TopLevelWindow * ComponentSearch::findWindowWithId (const juce::String & id)
{
    auto topWindows = getTopLevelWindows ();
    if (topWindows.empty ())
        return nullptr;

    if (id.isEmpty ())
        return topWindows.front ();

    auto it = std::find_if (
        topWindows.begin (),
        topWindows.end (),
        [&] (auto && window)
        { return window->getProperties ().getWithDefault (windowId, {}).toString () == id; });

    return it == topWindows.end () ? nullptr : *it;
}

int ComponentSearch::countChildComponents (const juce::Component & root,
                                           const juce::String & componentId)
{
    const auto predicate = createComponentMatcher (componentId);

    return std::accumulate (root.getChildren ().begin (),
                            root.getChildren ().end (),
                            0,
                            [=] (auto && total, auto && child)
                            {
                                if (predicate (*child))
                                    ++total;

                                total += countChildComponents (*child, componentId);

                                return total;
                            });
}

juce::Component * ComponentSearch::findWithId (const juce::String & componentId, int skip)
{
    return findComponent (createComponentMatcher (componentId), skip);
}

void ComponentSearch::setTestId (juce::Component & component, const juce::String & id)
{
    component.getProperties().set (testId, id);
}

}