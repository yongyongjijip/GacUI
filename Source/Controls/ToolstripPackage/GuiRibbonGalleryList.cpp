#include "GuiRibbonGalleryList.h"
#include "GuiRibbonImpl.h"
#include "../Templates/GuiThemeStyleFactory.h"

/* CodePack:BeginIgnore() */
#ifndef VCZH_DEBUG_NO_REFLECTION
/* CodePack:ConditionOff(VCZH_DEBUG_NO_REFLECTION, ../Reflection/TypeDescriptors/GuiReflectionPlugin.h) */
#include "../../Reflection/TypeDescriptors/GuiReflectionPlugin.h"
#endif
/* CodePack:EndIgnore() */

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			using namespace reflection::description;
			using namespace collections;
			using namespace compositions;

			namespace list
			{

/***********************************************************************
list::GalleryGroup
***********************************************************************/

				WString GalleryGroup::GetName()
				{
					return name;
				}

				Ptr<IValueList> GalleryGroup::GetItemValues()
				{
					return itemValues;
				}

/***********************************************************************
list::GroupedDataSource
***********************************************************************/
				void GroupedDataSource::RebuildItemSource()
				{
					if (!itemSource)
					{
						joinedItemSource = nullptr;
						groupedItemSource.Clear();
					}
					else if (GetGroupEnabled())
					{
						throw 0;
					}
					else
					{
						joinedItemSource = itemSource.Cast<IValueList>();
						if (!joinedItemSource)
						{
							joinedItemSource = IValueList::Create(GetLazyList<Value>(itemSource));
						}

						groupedItemSource.Clear();
						auto group = MakePtr<GalleryGroup>();
						group->itemValues = joinedItemSource;
						groupedItemSource.Add(group);
					}

					OnJoinedItemSourceChanged(joinedItemSource);
					OnGroupedItemSourceChanged(groupedItemSource.GetWrapper());
				}

				GroupedDataSource::GroupedDataSource(compositions::GuiGraphicsComposition* _associatedComposition)
					:associatedComposition(_associatedComposition)
				{
					GroupEnabledChanged.SetAssociatedComposition(associatedComposition);
					GroupTitlePropertyChanged.SetAssociatedComposition(associatedComposition);
					GroupChildrenPropertyChanged.SetAssociatedComposition(associatedComposition);
				}

				GroupedDataSource::~GroupedDataSource()
				{
				}

				Ptr<IValueEnumerable> GroupedDataSource::GetItemSource()
				{
					return itemSource;
				}

				void GroupedDataSource::SetItemSource(Ptr<IValueEnumerable> value)
				{
					if (itemSource != value)
					{
						itemSource = value;
						RebuildItemSource();
					}
				}

				bool GroupedDataSource::GetGroupEnabled()
				{
					return titleProperty && childrenProperty;
				}

				ItemProperty<WString> GroupedDataSource::GetGroupTitleProperty()
				{
					return titleProperty;
				}

				void GroupedDataSource::SetGroupTitleProperty(const ItemProperty<WString>& value)
				{
					if (titleProperty != value)
					{
						titleProperty = value;
						GroupTitlePropertyChanged.Execute(GuiEventArgs(associatedComposition));
						GroupEnabledChanged.Execute(GuiEventArgs(associatedComposition));
						RebuildItemSource();
					}
				}

				ItemProperty<Ptr<IValueEnumerable>> GroupedDataSource::GetGroupChildrenProperty()
				{
					return childrenProperty;
				}

				void GroupedDataSource::SetGroupChildrenProperty(const ItemProperty<Ptr<IValueEnumerable>>& value)
				{
					if (childrenProperty != value)
					{
						childrenProperty = value;
						GroupChildrenPropertyChanged.Execute(GuiEventArgs(associatedComposition));
						GroupEnabledChanged.Execute(GuiEventArgs(associatedComposition));
						RebuildItemSource();
					}
				}

				const GroupedDataSource::GalleryGroupList& GroupedDataSource::GetGroups()
				{
					return groupedItemSource;
				}
			}

/***********************************************************************
GuiBindableRibbonGalleryList
***********************************************************************/

			void GuiBindableRibbonGalleryList::BeforeControlTemplateUninstalled_()
			{
			}

			void GuiBindableRibbonGalleryList::AfterControlTemplateInstalled_(bool initialize)
			{
				auto ct = GetControlTemplateObject();
				itemList->SetControlTemplate(ct->GetItemListTemplate());
				subMenu->SetControlTemplate(ct->GetMenuTemplate());
				UpdateLayoutSizeOffset();
			}

			void GuiBindableRibbonGalleryList::UpdateLayoutSizeOffset()
			{
				auto cSize = itemList->GetContainerComposition()->GetBounds();
				auto bSize = itemList->GetBoundsComposition()->GetBounds();
				layout->SetSizeOffset(Size(bSize.Width() - cSize.Width(), bSize.Height() - cSize.Height()));
			}

			void GuiBindableRibbonGalleryList::OnJoinedItemSourceChanged(Ptr<IValueEnumerable> source)
			{
				itemList->SetItemSource(source);
			}

			void GuiBindableRibbonGalleryList::OnGroupedItemSourceChanged(Ptr<IValueEnumerable> source)
			{
			}

			void GuiBindableRibbonGalleryList::OnBoundsChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				UpdateLayoutSizeOffset();
				subMenu->GetBoundsComposition()->SetPreferredMinSize(Size(boundsComposition->GetBounds().Width(), 1));
			}

			void GuiBindableRibbonGalleryList::OnRequestedDropdown(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				subMenu->ShowPopup(this, Point(0, 0));
			}

			void GuiBindableRibbonGalleryList::OnRequestedScrollUp(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				itemListArranger->ScrollUp();
			}

			void GuiBindableRibbonGalleryList::OnRequestedScrollDown(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				itemListArranger->ScrollDown();
			}

			GuiBindableRibbonGalleryList::GuiBindableRibbonGalleryList(theme::ThemeName themeName)
				:GuiRibbonGallery(themeName)
				, GroupedDataSource(boundsComposition)
			{
				ItemTemplateChanged.SetAssociatedComposition(boundsComposition);
				SelectionChanged.SetAssociatedComposition(boundsComposition);

				layout = new ribbon_impl::GalleryResponsiveLayout;
				layout->SetAlignmentToParent(Margin(0, 0, 0, 0));
				containerComposition->AddChild(layout);

				itemListArranger = new ribbon_impl::GalleryItemArranger(this);
				itemList = new GuiBindableTextList(theme::ThemeName::RibbonGalleryItemList);
				itemList->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
				itemList->SetArranger(itemListArranger);
				layout->AddChild(itemList->GetBoundsComposition());

				subMenu = new GuiRibbonToolstripMenu(theme::ThemeName::RibbonToolstripMenu, this);

				RequestedScrollUp.AttachMethod(this, &GuiBindableRibbonGalleryList::OnRequestedScrollUp);
				RequestedScrollDown.AttachMethod(this, &GuiBindableRibbonGalleryList::OnRequestedScrollDown);
				RequestedDropdown.AttachMethod(this, &GuiBindableRibbonGalleryList::OnRequestedDropdown);
				boundsComposition->BoundsChanged.AttachMethod(this, &GuiBindableRibbonGalleryList::OnBoundsChanged);

				itemListArranger->UnblockScrollUpdate();
			}

			GuiBindableRibbonGalleryList::~GuiBindableRibbonGalleryList()
			{
				delete subMenu;
			}

			GuiBindableRibbonGalleryList::ItemStyleProperty GuiBindableRibbonGalleryList::GetItemTemplate()
			{
				return itemStyle;
			}

			void GuiBindableRibbonGalleryList::SetItemTemplate(ItemStyleProperty value)
			{
				if (itemStyle != value)
				{
					itemStyle = value;
					itemList->SetItemTemplate(value);
					ItemTemplateChanged.Execute(GetNotifyEventArguments());
				}
			}

			vint GuiBindableRibbonGalleryList::GetMinCount()
			{
				return layout->GetMinCount();
			}

			void GuiBindableRibbonGalleryList::SetMinCount(vint value)
			{
				layout->SetMinCount(value);
			}

			vint GuiBindableRibbonGalleryList::GetMaxCount()
			{
				return layout->GetMaxCount();
			}

			void GuiBindableRibbonGalleryList::SetMaxCount(vint value)
			{
				layout->SetMaxCount(value);
			}

			GalleryPos GuiBindableRibbonGalleryList::GetSelection()
			{
				throw 0;
			}

			void GuiBindableRibbonGalleryList::SetSelection(GalleryPos value)
			{
				throw 0;
			}

			GuiToolstripMenu* GuiBindableRibbonGalleryList::GetSubMenu()
			{
				return subMenu;
			}
		}
	}
}