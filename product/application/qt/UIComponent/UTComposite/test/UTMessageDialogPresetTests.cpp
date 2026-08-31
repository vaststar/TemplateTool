#include <catch2/catch_test_macros.hpp>

#include <UTMessageDialog/UTMessageButtonPresets.h>
#include <UTMessageDialog/UTMessagePresets.h>

#include <optional>

namespace {

void requireButtonSelectionSemantics(const UTMessageOptions& options,
                                     std::optional<int> expectedDefaultIndex,
                                     std::optional<int> expectedCancelIndex)
{
    std::optional<int> defaultIndex;
    std::optional<int> cancelIndex;

    for (int index = 0; index < options.buttons.size(); ++index)
    {
        const auto& button = options.buttons.at(index);
        if (button.isDefault)
        {
            REQUIRE_FALSE(defaultIndex.has_value());
            defaultIndex = index;
        }
        if (button.isCancel)
        {
            REQUIRE_FALSE(cancelIndex.has_value());
            cancelIndex = index;
        }
    }

    REQUIRE(defaultIndex == expectedDefaultIndex);
    REQUIRE(cancelIndex == expectedCancelIndex);
}

} // namespace

TEST_CASE("Message button presets distinguish acknowledgement and OK actions",
          "[UTMessageDialog][Presets]")
{
    const auto acknowledgement = UTMessageButtonPresets::acknowledge();
    REQUIRE(acknowledgement.role == UTButtonRole::Accept);
    REQUIRE(acknowledgement.isDefault);
    REQUIRE(acknowledgement.isCancel);

    const auto ok = UTMessageButtonPresets::ok();
    REQUIRE(ok.role == UTButtonRole::Accept);
    REQUIRE(ok.isDefault);
    REQUIRE_FALSE(ok.isCancel);

    const auto cancel = UTMessageButtonPresets::cancel();
    REQUIRE(cancel.role == UTButtonRole::Reject);
    REQUIRE_FALSE(cancel.isDefault);
    REQUIRE(cancel.isCancel);
}

TEST_CASE("Single-button message presets use one acknowledgement action",
          "[UTMessageDialog][Presets]")
{
    const auto requireAcknowledgement = [](const UTMessageOptions& options)
    {
        REQUIRE(options.buttons.size() == 1);
        requireButtonSelectionSemantics(options, 0, 0);
    };

    requireAcknowledgement(
        UTMessagePresets::info(QStringLiteral("title"), QStringLiteral("message")));
    requireAcknowledgement(
        UTMessagePresets::warning(QStringLiteral("title"), QStringLiteral("message")));
    requireAcknowledgement(
        UTMessagePresets::error(QStringLiteral("title"), QStringLiteral("message")));
    requireAcknowledgement(
        UTMessagePresets::success(QStringLiteral("title"), QStringLiteral("message")));
}

TEST_CASE("Confirmation presets expose unique default and cancel actions",
          "[UTMessageDialog][Presets]")
{
    const auto confirm = UTMessagePresets::confirm(
        QStringLiteral("title"), QStringLiteral("message"));
    REQUIRE(confirm.buttons.size() == 2);
    requireButtonSelectionSemantics(confirm, 0, 1);

    const auto okCancel = UTMessagePresets::okCancel(
        QStringLiteral("title"), QStringLiteral("message"));
    REQUIRE(okCancel.buttons.size() == 2);
    requireButtonSelectionSemantics(okCancel, 0, 1);

    const auto destructive = UTMessagePresets::destructiveConfirm(
        QStringLiteral("title"),
        QStringLiteral("message"),
        QStringLiteral("Delete"));
    REQUIRE(destructive.buttons.size() == 2);
    requireButtonSelectionSemantics(destructive, std::nullopt, 0);
}
